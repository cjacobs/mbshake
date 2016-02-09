#include "vec3.h"
#include "delayBuffer.h"
#include "runningStats.h"
#include "eventThresholdFilter.h"
#include "iirFilter.h"
#include "classifiers.h"
#include "main.h"

#include <array>
#include <cmath>
#include <cstdlib>
using std::abs;
using std::max;

// TODO:
// * Maybe modulate the shake output slightly by the amount of energy? --- soft shakes
//     return a very large prediction value --- often higher than a strong shake
// * Maybe tune shake frequency by energy? hard shakes are somewhat slower (are they?)
// * Maybe use max over some window instead of mean for shake pred value? (though this
//     risks making transitory spikes last longer and be harder to filter out

// TODO: still doesn't detect taps if device is anchored to table. Then, the var over the big window is [0,0,0,0,0... ~12, ...]
// Maybe check if var over an even bigger window is exactly(ish) 0, and lower the threshold even more if so?


// Constants
const float lowpassFilterCoeff = 1.0f;
const float gravityFilterCoeff = 0.005f; // problem: gravity converges too slowly (?)
const float gravityFilterThresh = 126;

const float minLenThresh = 5; 

const float shakeGestureThreshold = 0.4f;
const int shakeEventCountThreshold = 5;
const int shakeEventCountLowThreshold = 2;
const float shakeGateThreshSquared = 4; //000000.0f;
eventThresholdFilter<float> shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold, shakeEventCountLowThreshold);

// Tap stuff
const float tapGestureThreshold = 200.0f; // TODO: this can be an int
const int tapEventCountThreshold = 1;

const int g_tapLargeWindowSize = 8; //11; // maybe too big?
const int g_tapImpulseWindowSize = 2;
const float g_tapScaleDenominator = 2.5f;
const float tapGateThresh1 = 25.0f; // variance of preceeding windown should be less than this
const int g_tapK = 2;

const int shakeStatsBufferSize = 10;

const int dotWavelength1 = 5;
const int dotWavelength2 = 6;
const int dotWavelength3 = 7;
const int dotWavelength4 = 4;

const int dotMeanWindow1 = dotWavelength1 / 2;
const int dotMeanWindow2 = dotWavelength2 / 2;
const int dotMeanWindow3 = dotWavelength3 / 2;
const int dotMeanWindow4 = dotWavelength4 / 2;

// const int delayBufferSize = 2*(dotWavelength2) + shakeStatsBufferSize;
const int delayBufferSize = 33; // TODO: eventually find correct minimum size here

// Globals
floatVec3 g_gravity = {0, 0, 0};

// filters
// TODO: make simple_iirFilter for single-pole filter
iirFilter<floatVec3, 1, 1> lowpassFilter({ 1.0f-lowpassFilterCoeff }, { -lowpassFilterCoeff });
iirFilter<floatVec3, 1, 1> gravityFilter({ 1.0f-gravityFilterCoeff }, { -gravityFilterCoeff });

// Global delay buffer for filtered, gravity-subtracted accel input
delayBuffer<byteVec3, delayBufferSize> g_sampleDelay;

// Tap gesture stats
// windowed statistics for detecting quiet area before tap
runningStats<g_tapLargeWindowSize, delayBufferSize, long, byteVec3, GetZ<int8_t>> g_tapLargeWindowStats(g_sampleDelay);

// windowed statistics for detecting high-Z-energy area during tap
runningStats<g_tapImpulseWindowSize, delayBufferSize, long, byteVec3, GetZ<int8_t>> g_tapImpulseWindowStats(g_sampleDelay);

eventThresholdFilter<float> tapEventFilter(tapGestureThreshold, tapEventCountThreshold, 0);

// Shake gesture stats
runningStats<shakeStatsBufferSize, delayBufferSize, float, byteVec3, GetMagSq<int8_t, float>> g_shakeThreshStats(g_sampleDelay);


// TODO: quantize these to shorts or something
delayBuffer<float, dotMeanWindow1 + 1> g_dotDelay1;
auto g_delayDot1Stats = makeStats(g_dotDelay1);

delayBuffer<float, dotMeanWindow2 + 1> g_dotDelay2;
auto g_delayDot2Stats = makeStats(g_dotDelay2);

delayBuffer<float, dotMeanWindow3 + 1> g_dotDelay3;
auto g_delayDot3Stats = makeStats(g_dotDelay3);

delayBuffer<float, dotMeanWindow4 + 1> g_dotDelay4;
auto g_delayDot4Stats = makeStats(g_dotDelay4);

// perp delay
delayBuffer<float, dotWavelength1 + 1> g_perpDelay1;
auto g_delayPerp1Stats = makeStats(g_perpDelay1);

int g_tapCountdown1 = 0;
float g_prevQuietVar = 0;
delayBuffer<float, g_tapK+1> g_quietVarDelay;

// !!! Can we subsample a sequence at compile time using template metaprogramming?

// templates
const byteVec3 shakeTemplate1[] = {{ -10,    7,  -91},
                                   {  43,    0, -102},
                                   { 107,  -37, -101},
                                   { 109,    5,  -75},
                                   {  98,   56,  -66},
                                   {   9,   97,  -36},
                                   { -67,   78,  -65},
                                   { -86,   40, -127},
                                   { -87,   55, -103},
                                   { -29,   64,  -97},
                                   {  43,   64,  -84},
                                   {  81,   28,  -82},
                                   { 108,  -41,  -78},
                                   {  76,  -13,  -69},
                                   {  10,    3,  -46},
                                   {  -2,   -1,  -49}};

// need a templateDist function that takes a template, delay line, and resample rate, and returns the distance between the template and signal represented by the delay line, subsampled by the resample rate

// Note: tmpl should be time-reversed
template <int N, int ResampleRate, typename T, typename U>
float templateDistSq(T* tmpl, delayBuffer<U, N>& signal)
{
    // TODO: can probably do all the following with template metaprogramming if we really care
    float result;
    for(int index = 0; index < N; index++)
    {
        auto temp = tmpl[N-index-1] - signal.getDelayedSample(ResampleRate*index);
        auto tempNormSq = normSq(temp);
        result += tempNormSq;
    }
    return result;
}

template <typename Fn>
class Debouncer
{
public:
    Debouncer(Fn buttonFn) : _counter(0), _didPush(false), _buttonFn(buttonFn)
    {
        _counter = 0;
        _didPush = false;
    }

    bool operator()()
    {
        if(_buttonFn())
        {
            ++_counter;
            if(_counter >= upThresh)
            {
                _didPush = true;
                _counter = upThresh;
            }
        }
        else
        {
            if(_counter > 0)
            {
                _counter--;
                if(_counter <= downThresh)
                {
                    if(_didPush)
                    {
                        _didPush = false;
                        _counter = 0;
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
private:
    int _counter = 0;
    bool _didPush = false;
    const int upThresh = 4;
    const int downThresh = 2;
    Fn _buttonFn;
};

template <typename Fn>
auto makeDebouncer(Fn fn) -> Debouncer<Fn>
{
    return Debouncer<Fn>(fn);
}

auto buttonAUp = makeDebouncer(buttonA);
auto buttonBUp = makeDebouncer(buttonB);

bool isPrinting = false;
bool printShake = true;

byteVec3 quantizeSample(const byteVec3& b, int factor)
{
    // TODO: round appropriately
    return b / float(factor); // TODO: shouldn't we scale back up?
}

// For some reason, this kills the micro:bit for a while
template<typename MeanDelayType, typename MeanStatsType>
void processDotFeature(const byteVec3& currentSample, int dotWavelength, MeanDelayType& meanDelay, MeanStatsType& delayDotStats)
{
    // TODO: investigate if this really help like it appears to do in the python version
    int quantRate = 16;

    // holy crap... in the python code, we were severely clipping the data
    byteVec3 quantizedCurrentSample = quantizeSample(currentSample, 1);

    //    if(buttonA())
    //        serialPrint("q: ", quantizedCurrentSample, "  ");

    float dot1a = dotNorm(quantizedCurrentSample, quantizeSample(g_sampleDelay.getDelayedSample(dotWavelength), quantRate), minLenThresh);
    float dot1b = dotNorm(quantizedCurrentSample, quantizeSample(g_sampleDelay.getDelayedSample(2 * dotWavelength), quantRate), minLenThresh);

    if (dot1a < 0 && dot1b > 0)
    {
        meanDelay.addSample(dot1b - dot1a);
        delayDotStats.addSample(dot1b - dot1a);
    }
    else
    {
        meanDelay.addSample(0.0);
        delayDotStats.addSample(0.0);
    }
}

float getShakePrediction();
float getTapPrediction();

byteVec3 g_lastRawSample;
void processSample(byteVec3 sample)
{
    g_lastRawSample = sample;
    //*
    static floatVec3 filteredSample(sample);
    filterVec(floatVec3(sample), filteredSample, lowpassFilterCoeff);
    filterVec(filteredSample, g_gravity, gravityFilterCoeff, gravityFilterThresh);

    /*
    floatVec3 filteredSample = lowpassFilter.filterSample(floatVec3(sample));
    g_gravity = gravityFilter.filterSample(filteredSample);
    // */
    
    byteVec3 currentSample = byteVec3 { clampByte(sample.x-g_gravity.x),
                                        clampByte(sample.y-g_gravity.y),
                                        clampByte(sample.z-g_gravity.z) };
    
    // TODO: maybe we should somehow associate the stats objects with the delay lines
    //       then we won't have to remember to add the stats.addSample() lines
    g_sampleDelay.addSample(currentSample); 
    g_tapLargeWindowStats.addSample(currentSample);
    g_tapImpulseWindowStats.addSample(currentSample);
    g_shakeThreshStats.addSample(currentSample);
    
    // now add val to mean buffer
    //        processDotFeature(currentSample, dotWavelength1, g_dotDelay1, g_delayDot1Stats);
    processDotFeature(currentSample, dotWavelength2, g_dotDelay2, g_delayDot2Stats);
    processDotFeature(currentSample, dotWavelength3, g_dotDelay3, g_delayDot3Stats);
    //        processDotFeature(currentSample, dotWavelength4, g_dotDelay4, g_delayDot4Stats);
    
    if(isPrinting)
    {
        /*
        if(printShake)
        {
            // serialPrintLn(systemTime(), " : ", g_lastRawSample, " :: ", getShakePrediction(), (g_shakeThreshStats.getVar() > shakeGateThreshSquared) ? " ##" : "  ");
            auto shakePred = getShakePrediction();
            serialPrintLn(systemTime(), " : ", currentSample, " :: ", shakePred, (shakePred > shakeGestureThreshold) ? " ##" : "  ");
        }
        else
        {
            auto tapPred = getTapPrediction();
            auto quietStuff = g_tapLargeWindowStats.getVar();
            serialPrintLn(systemTime(), " : ", currentSample, " :: ", tapPred, "\t", quietStuff, (tapPred > tapGestureThreshold) ? " ## " : "  ", g_tapCountdown1);
        }
        */
    }
}

float getShakePrediction()
{    
    return g_delayDot2Stats.getMean();
    // return max(g_delayDot2Stats.getMean(), g_delayDot3Stats.getMean());
}

float getTapPrediction()
{
    // If previous quiet window was very very quiet (e.g., 0), then
    // increase output (when micro:bit is sitting on table, tap
    // amplitude is diminished)

    float scale = fast_inv_sqrt(1.0 + g_quietVarDelay.getDelayedSample(g_tapK));
    return g_tapImpulseWindowStats.getVar() * scale;
}


void initClassifiers()
{
    // init gravity
    updateAccelerometer();
    g_gravity = floatVec3(getAccelData()); // UGH
}

int detectGesture()
{
    if(buttonAUp())
    {
        printShake = !printShake;
    }

    if(buttonBUp())
    {
        isPrinting = !isPrinting;
    }

    updateAccelerometer();
    byteVec3 sample = getAccelData();

    bool shouldCheckTap = g_tapCountdown1 > 0;
    bool shouldCheckShake = g_shakeThreshStats.getVar() > shakeGateThreshSquared;

    // criterion 1: look for N samples worth of quiet
    float quietVariance = g_tapLargeWindowStats.getVar();
    g_quietVarDelay.addSample(quietVariance);

    if (quietVariance <= tapGateThresh1)
    {
        g_tapCountdown1 = g_tapK;
        g_prevQuietVar = quietVariance;
    }
    else if(g_tapCountdown1 > 0)
    {
        g_tapCountdown1 -= 1;
    }

    processSample(sample);

    if(shouldCheckTap)
    {
        float tapPredVal = getTapPrediction();
        bool foundTap = tapEventFilter.filterValue(tapPredVal);
        if(foundTap)
        {
            shakeEventFilter.reset();
            if(isPrinting)
            {
                serialPrintLn("Tap\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", g_lastRawSample, "\t", getShakePrediction());
            }
            return MICROBIT_ACCELEROMETER_TAP;
        }
    }

    if(shouldCheckShake)
    {
        float shakePredVal = getShakePrediction();
        //    float tmplDist = templateDistSq<16, 2>(shakeTemplate1, g_sampleDelay);
        //    serialPrint("tmpl: ", tmplDist);
        //serialPrint("accel: ", g_sampleDelay.getDelayedSample(0));

        bool foundShake = shakeEventFilter.filterValue(shakePredVal);
        if(foundShake)
        {
            tapEventFilter.reset();
            if(isPrinting)
            {
                serialPrintLn("Shake\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", g_lastRawSample, "\t", getShakePrediction());
            }
            return MICROBIT_ACCELEROMETER_SHAKE;
        }
    }
    else
    {
        shakeEventFilter.reset();
    }

    if(isPrinting)
    {
        serialPrintLn("\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", g_lastRawSample, "\t", getShakePrediction());
    }
    return 0;
}
