#include "vec3.h"
#include "delayBuffer.h"
#include "runningStats.h"
#include "eventThresholdFilter.h"
#include "iirFilter.h"
#include "classifiers.h"
#include "main.h"

// TODO:
// * Maybe modulate the shake output slightly by the amount of energy? --- soft shakes
//     return a very large prediction value --- often higher than a strong shake
// * Maybe tune shake frequency by energy? hard shakes are somewhat slower (are they?)
// * Maybe use max over some window instead of mean for shake pred value? (though this
//     risks making transitory spikes last longer and be harder to filter out

// Constants
const float lowpassFilterCoeff = 0.9;
const float gravityFilterCoeff = 0.05; // problem: gravity converges too slowly
const int minLenThresh = 25; //150*150;

const float shakeGestureThreshold = 0.65;
const int shakeEventCountThreshold = 3;
const float shakeGateThreshSquared = 1600000;
eventThresholdFilter shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold);

const float tapGestureThreshold = 1.7;
const int tapEventCountThreshold = 1;

const int g_tapWindowSize = 11;
const float g_tapScaleDenominator = 2.5;
const float tapGateThresh1 = 500; // variance of preceeding windown should be less than this
const int tapGateThresh2 = 120; // intensity of impulse should be greater than this
const int g_tapK = 3;

const int meanBufferSize = 8;

const int dotWavelength1 = 6;
const int dotWavelength2 = 8;

// const int delayBufferSize = 2*(dotWavelength2) + meanBufferSize;
const int delayBufferSize = 33;

// TODO: half-phase delayed versions

// Globals
floatVec3 g_gravity = {0,0,0};

byteVec3 g_sampleDelayMem[delayBufferSize];
delayBuffer<byteVec3> g_sampleDelay(g_sampleDelayMem, delayBufferSize);
runningStats<long, byteVec3, GetZ<int8_t>> g_tapStats(g_tapWindowSize, g_sampleDelay);
eventThresholdFilter tapEventFilter(tapGestureThreshold, tapEventCountThreshold);

runningStats<float, byteVec3, GetMagSq<int8_t, float>> g_shakeThreshStats(meanBufferSize, g_sampleDelay);

// TODO: quantize this to a short or something
float g_meanDelay1Mem[meanBufferSize+1];
delayBuffer<float> g_meanDelay1(g_meanDelay1Mem, meanBufferSize+1);
runningStats<float> g_delayDot1Stats(meanBufferSize, g_meanDelay1);

// TODO: quantize this to a short or something
float g_meanDelay2Mem[meanBufferSize+1];
delayBuffer<float> g_meanDelay2(g_meanDelay2Mem, meanBufferSize+1);
runningStats<float> g_delayDot2Stats(meanBufferSize, g_meanDelay2);


int g_tapCountdown1 = 0;
int g_tapCountdown2 = 0;

//float g_meanDelay2Mem[meanBufferSize];
//delayBuffer<float> g_meanDelay2(g_meanDelay2Mem, meanBufferSize);


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
float templateDistSq(T* tmpl, delayBuffer<U>& signal)
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


// TODO: use iirFilter object here
void filterVec(const floatVec3& vec, floatVec3& prevVec, float alpha)
{
    prevVec.x += alpha*(vec.x - prevVec.x);
    prevVec.y += alpha*(vec.y - prevVec.y);
    prevVec.z += alpha*(vec.z - prevVec.z);
}



template <typename T>
T sum(const T* buf, int len)
{
    const T* end = buf+len;
    T result = 0;
    while(buf != end)
    {
        result += *buf++;
    }
    return result;
}

void processSample(byteVec3 sample)
{
    floatVec3 filteredSample;
    filteredSample.x = sample.x;
    filteredSample.y = sample.y;
    filteredSample.z = sample.z;

    filterVec(floatVec3(sample), filteredSample, lowpassFilterCoeff);
    filterVec(filteredSample, g_gravity, gravityFilterCoeff);
    byteVec3 currentSample = byteVec3 { clampByte(sample.x-g_gravity.x),
                                        clampByte(sample.y-g_gravity.y),
                                        clampByte(sample.z-g_gravity.z) };
    
    // TODO: maybe we should somehow associate the stats objects with the delay lines
    //       then we won't have to remember to add the stats.addSample() lines
    g_sampleDelay.addSample(currentSample); 
    g_tapStats.addSample(currentSample);
    g_shakeThreshStats.addSample(currentSample);
    
    // now add val to mean buffer
    float dot1a = dotNorm(currentSample, g_sampleDelay.getDelayedSample(dotWavelength1), minLenThresh);
    float dot1b = dotNorm(currentSample, g_sampleDelay.getDelayedSample(2*dotWavelength1), minLenThresh);
    if(dot1a < 0 && dot1b > 0)
    {
        g_meanDelay1.addSample(dot1b-dot1a);
        g_delayDot1Stats.addSample(dot1b-dot1a);
    }
    else
    {
        g_meanDelay1.addSample(0.0);
        g_delayDot1Stats.addSample(0.0);
    }

    float dot2a = dotNorm(currentSample, g_sampleDelay.getDelayedSample(dotWavelength2), minLenThresh);
    float dot2b = dotNorm(currentSample, g_sampleDelay.getDelayedSample(2*dotWavelength2), minLenThresh);
    if(dot2a < 0 && dot2b > 0)
    {
        g_meanDelay2.addSample(dot2b-dot2a);
        g_delayDot2Stats.addSample(dot2b-dot2a);
    }
    else
    {
        g_meanDelay2.addSample(0.0);
        g_delayDot2Stats.addSample(0.0);
    }
}

float getShakePrediction()
{    
    return std::max(g_delayDot1Stats.getMean(), g_delayDot2Stats.getMean());
}

float getTapPrediction()
{
  float val1 = g_sampleDelay.getDelayedSample(g_tapWindowSize/2).z - g_tapStats.getMean();
  float val2 = g_sampleDelay.getDelayedSample(g_tapWindowSize/2 - 1).z - g_tapStats.getMean();
  float val = val1 - 0.25*val2;
  float std = g_tapStats.getStdDev();
  
  return val / (std + g_tapScaleDenominator);
}


void initClassifiers()
{
    // init gravity
    updateAccelerometer();
    g_gravity = floatVec3(getAccelData()); // UGH
}

int detectGesture()
{
    updateAccelerometer();
    byteVec3 sample = getAccelData();

    bool shouldCheckTap = g_tapCountdown1 > 0 && g_tapCountdown2 > 0;
    bool shouldCheckShake = g_shakeThreshStats.getVar() > shakeGateThreshSquared;

    // criterion 1: look for 5 samples worth of quiet
    float tapVar = g_tapStats.getVar();

    if (tapVar <= tapGateThresh1)
    {
        g_tapCountdown1 = g_tapWindowSize + g_tapK;
    }
    else if(g_tapCountdown1 > 0)
    {
        g_tapCountdown1 -= 1;
    }

    int zDiff = abs(sample.z - g_sampleDelay.getDelayedSample(2).z);
    if (zDiff >= tapGateThresh2)
    {
        g_tapCountdown2 = (g_tapWindowSize/2) + g_tapK;
    }
    else if(g_tapCountdown2 > 0)
    {
        g_tapCountdown2 -= 1;
    }

    processSample(sample);

    if(shouldCheckTap)
    {
        float tapPredVal = getTapPrediction();
        bool foundTap = tapEventFilter.filterValue(tapPredVal);
        if(foundTap)
        {
            shakeEventFilter.reset();
            return MICROBIT_ACCELEROMETER_TAP;
        }
    }


    if(shouldCheckShake)
    {
        float shakePredVal = getShakePrediction();
        // serialPrint("shake: ", shakePredVal);
        //    float tmplDist = templateDistSq<16, 2>(shakeTemplate1, g_sampleDelay);
        //    serialPrint("tmpl: ", tmplDist);
        serialPrint("accel: ", g_sampleDelay.getDelayedSample(0));

        bool foundShake = shakeEventFilter.filterValue(shakePredVal);
        if(foundShake)
        {
            tapEventFilter.reset();
            return MICROBIT_ACCELEROMETER_SHAKE;
        }
    }
    else
    {
        shakeEventFilter.reset();
    }
    return 0;
}
