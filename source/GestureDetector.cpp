#include "Vector3.h"
#include "DelayBuffer.h"
#include "RunningStats.h"
#include "EventThresholdFilter.h"
#include "IirFilter.h"
#include "GestureDetector.h"
#include "MicroBitAccess.h"

#include <cmath>
#include <cstdlib>
using std::abs;
using std::max;

const int sampleRate = 18; // in ms

const float lowpassFilterCoeff = 0.8f;
const float gravityFilterCoeff = 0.005f; // problem: gravity converges too slowly (?)
const float gravityFilterThresh = 126;

const float minLenThresh = 1; 

const float shakeGestureThreshold = 0.4f;
const int shakeEventCountThreshold = 6;
const int shakeEventCountLowThreshold = 3;
const float shakeGateThreshSquared = 4; //000000.0f;

// Tap stuff
const float tapGestureThreshold = 200.0f; // TODO: this can be an int
const int tapEventCountThreshold = 1;

const float tapScaleDenominator = 2.5f;
const float tapGateThresh1 = 25.0f; // variance of preceeding windown should be less than this


// TODO:
// * Maybe modulate the shake output slightly by the amount of energy? --- soft shakes
//     return a very large prediction value --- often higher than a strong shake
// * Maybe tune shake frequency by energy? hard shakes are somewhat slower (are they?)
// * Maybe use max over some window instead of mean for shake pred value? (though this
//     risks making transitory spikes last longer and be harder to filter out

// TODO: still doesn't detect taps if device is anchored to table. Then, the var over the big window is [0,0,0,0,0... ~12, ...]
// Maybe check if var over an even bigger window is exactly(ish) 0, and lower the threshold even more if so?


// !!! Can we subsample a sequence at compile time using template metaprogramming?

// templates
const byteVector3 shakeTemplate1[] = {{ -10,    7,  -91},
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
float templateDistSq(T* tmpl, DelayBuffer<U, N>& signal)
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

GestureDetector::GestureDetector() : gravityFilter(gravityFilterCoeff),
                                     tapLargeWindowStats(sampleDelayBuffer),
                                     tapImpulseWindowStats(sampleDelayBuffer),
                                     shakeThreshStats(sampleDelayBuffer),
                                     dot1Stats(dotDelayBuffer1),
                                     dot2Stats(dotDelayBuffer2),
                                     dot3Stats(dotDelayBuffer3),
                                     dot4Stats(dotDelayBuffer4),
    shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold, shakeEventCountLowThreshold),
    tapEventFilter(tapGestureThreshold, tapEventCountThreshold, 0)

    
{
    // init() // ?
}

void GestureDetector::init()
{
    // init gravity
    updateAccelerometer();
    auto sample = getAccelData();
    floatVector3 floatSample = floatVector3(sample);
    gravity = floatSample;
    filteredSample = floatSample;
    gravityFilter.init(floatSample);
}

byteVector3 quantizeSample(const byteVector3& b, int factor)
{
    // TODO: round appropriately, be more efficient
    return byteVector3(b / float(factor))*factor;
}

// For some reason, this kills the micro:bit for a while
template<typename MeanDelayType, typename MeanStatsType>
void GestureDetector::processDotFeature(const byteVector3& currentSample, int dotWavelength, MeanDelayType& meanDelay, MeanStatsType& delayDotStats)
{
    // TODO: investigate if this really help like it appears to do in the python version
    int quantRate = 16;

    // omigosh... in the python code, we were severely clipping the data
    byteVector3 quantizedCurrentSample = quantizeSample(currentSample, quantRate);

    float dot1a = dotNorm(quantizedCurrentSample, quantizeSample(sampleDelayBuffer.getDelayedSample(dotWavelength), quantRate), minLenThresh);
    float dot1b = dotNorm(quantizedCurrentSample, quantizeSample(sampleDelayBuffer.getDelayedSample(2 * dotWavelength), quantRate), minLenThresh);

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

void GestureDetector::processSample(byteVector3 sample)
{
    lastRawSample = sample;
    gravity = gravityFilter.filterSample(floatVector3(sample));
    
    byteVector3 currentSample = byteVector3 { clampByte(sample.x-gravity.x),
                                        clampByte(sample.y-gravity.y),
                                        clampByte(sample.z-gravity.z) };
    
    lastFilteredSample = currentSample;

    // TODO: maybe we should somehow associate the stats objects with the delay lines
    //       then we won't have to remember to add the stats.addSample() lines
    sampleDelayBuffer.addSample(currentSample); 
    tapLargeWindowStats.addSample(currentSample);
    tapImpulseWindowStats.addSample(currentSample);
    shakeThreshStats.addSample(currentSample);
    
    // now add val to mean buffer
    processDotFeature(currentSample, dotWavelength2, dotDelayBuffer2, dot2Stats);
    if(allowSlowGesture)
    {
        processDotFeature(currentSample, dotWavelength4, dotDelayBuffer4, dot4Stats);
    }
}

float GestureDetector::getShakePrediction()
{    
    if(allowSlowGesture)
    {
        return max(dot2Stats.getMean(), dot4Stats.getMean());
    }
    else
    {
        return dot2Stats.getMean();
    }
}

float GestureDetector::getTapPrediction()
{
    // If previous quiet window was very very quiet (e.g., 0), then
    // increase output (when micro:bit is sitting on table, tap
    // amplitude is diminished)

    float scale = fast_inv_sqrt(1.0 + quietVarDelay.getDelayedSample(tapK));
    return tapImpulseWindowStats.getVar() * scale;
}

void GestureDetector::togglePrinting()
{
    isPrinting = !isPrinting;
}

void GestureDetector::toggleAlg()
{
    allowSlowGesture = !allowSlowGesture;
    if(allowSlowGesture)
    {
        showChar('2', 50);
    }
    else
    {
        showChar('1', 50);
    }
}

void GestureDetector::systemTick()
{
    unsigned long time = systemTime();

    // If enough time has elapsed or the timer rolls over, do something
    if ((time-prevTime) >= sampleRate || time < prevTime) 
    {
        prevTime = time;
        state = detectGesture();
    }
}

int GestureDetector::detectGesture()
{
    updateAccelerometer();
    byteVector3 sample = getAccelData();
    
    bool shouldCheckTap = tapCountdown1 > 0;
    bool shouldCheckShake = shakeThreshStats.getVar() > shakeGateThreshSquared;

    // criterion 1: look for N samples worth of quiet
    float quietVariance = tapLargeWindowStats.getVar();
    quietVarDelay.addSample(quietVariance);

    if (quietVariance <= tapGateThresh1)
    {
        tapCountdown1 = tapK;
        prevQuietVar = quietVariance;
    }
    else if(tapCountdown1 > 0)
    {
        tapCountdown1 -= 1;
    }

    processSample(sample);

    float diagnosticVal = 0;
    if(isPrinting) diagnosticVal = getShakePrediction();

    if(shouldCheckTap)
    {
        float tapPredVal = getTapPrediction();
        bool foundTap = tapEventFilter.filterValue(tapPredVal);
        if(foundTap)
        {
            shakeEventFilter.reset();
            if(isPrinting)
            {
                serialPrintLn("Tap\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", lastRawSample, "\t", diagnosticVal);
            }
            return MICROBIT_ACCELEROMETER_TAP;
        }
    }

    if(shouldCheckShake)
    {
        float shakePredVal = getShakePrediction();
        bool foundShake = shakeEventFilter.filterValue(shakePredVal);
        if(foundShake)
        {
            tapEventFilter.reset();
            if(isPrinting)
            {
                serialPrintLn("Shake\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", lastRawSample, "\t", diagnosticVal);
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
        serialPrintLn("\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", lastRawSample, "\t", diagnosticVal);
    }

    return 0; // none
}

int GestureDetector::getCurrentGesture()
{
    return state;
}
