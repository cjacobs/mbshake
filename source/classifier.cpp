#include "vec3.h"
#include "delayBuffer.h"
#include "eventThresholdFilter.h"
#include "iirFilter.h"
#include "classifier.h"
#include "main.h"

// Constants
const float lowpassFilterCoeff = 0.9;
const float gravityFilterCoeff = 0.05;
const int minLenThresh = 25; //150*150;

const float shakeGestureThreshold = 0.65;
const int shakeEventCountThreshold = 3;
const float shakeGateThreshSquared = 1600000;
eventThresholdFilter shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold);

const float tapGestureThreshold = 1.7;
const int tapEventCountThreshold = 1;

const int g_tapWindowSize = 11;
const float g_tapScaleDenominator = 2.5;
const int tapGateThresh = 130;
const int g_tapK = 3;

const int meanBufferSize = 11;

const int dotWavelength1 = 6;
const int dotWavelength2 = 13;
const int delayBufferSize = 2*(dotWavelength2) + meanBufferSize;



// Globals
floatVec3 g_gravity = {0,0,0};

byteVec3 g_sampleDelayMem[delayBufferSize];
delayBuffer<byteVec3> g_sampleDelay(g_sampleDelayMem, delayBufferSize);
runningStats<long, byteVec3, GetZ<int8_t>> g_tapStats(g_tapWindowSize, g_sampleDelay);
eventThresholdFilter tapEventFilter(tapGestureThreshold, tapEventCountThreshold);

float g_meanDelay1Mem[meanBufferSize+1];
delayBuffer<float> g_meanDelay1(g_meanDelay1Mem, meanBufferSize+1);
runningStats<float> g_delayDotStats(meanBufferSize, g_meanDelay1);

runningStats<float, byteVec3, GetMagSq<int8_t, float>> g_shakeStats(meanBufferSize, g_sampleDelay);

int g_tapCountdown = 0;

//float g_meanDelay2Mem[meanBufferSize];
//delayBuffer<float> g_meanDelay2(g_meanDelay2Mem, meanBufferSize);

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
    filterVec(floatVec3(sample), filteredSample, lowpassFilterCoeff);
    filterVec(filteredSample, g_gravity, gravityFilterCoeff);
    byteVec3 currentSample = byteVec3 { clampByte(sample.x-g_gravity.x),
                                        clampByte(sample.y-g_gravity.y),
                                        clampByte(sample.z-g_gravity.z) };
    
    //    printf("sample:  %d\t%d\t%d\r\n", currentSample.x, currentSample.y, currentSample.z);
    //    printf("gravity: %d\t%d\t%d\r\n", int(g_gravity.x), int(g_gravity.y), int(g_gravity.z));
    //    printf("\r\n");

    g_sampleDelay.addSample(currentSample); // TODO: maybe we should somehow associate the stats objects with the delay lines
    g_tapStats.addSample(currentSample);
    g_shakeStats.addSample(currentSample);
    
    // now add val to mean buffer
    float dot1a = dotNorm(currentSample, g_sampleDelay.getDelayedSample(dotWavelength1), minLenThresh);
    float dot1b = dotNorm(currentSample, g_sampleDelay.getDelayedSample(2*dotWavelength1), minLenThresh);
    if(dot1a < 0 && dot1b > 0)
    {
        g_meanDelay1.addSample(dot1b-dot1a);
        g_delayDotStats.addSample(dot1b-dot1a);
    }
    else
    {
        g_meanDelay1.addSample(0.0);
        g_delayDotStats.addSample(0.0);
    }


    /*
    float dot2a = dotNorm(currentSample, g_sampleDelay.getDelayedSample(dotWavelength2), minLenThresh);
    float dot2b = dotNorm(currentSample, g_sampleDelay.getDelayedSample(2*dotWavelength2), minLenThresh);
    if(dot2a < 0 && dot2b > 0)
        g_meanDelay2.addSample(dot2b-dot2a);
    else
        g_meanDelay2.addSample(0.0); // ?
    */
}

float getShakePrediction()
{    
    return g_delayDotStats.getMean();
}

float getTapPrediction()
{
  float val1 = g_sampleDelay.getDelayedSample(g_tapWindowSize/2).z - g_tapStats.getMean();
  float val2 = g_sampleDelay.getDelayedSample(g_tapWindowSize/2 - 1).z - g_tapStats.getMean();
  float val = val1 - 0.25*val2;
  float std = g_tapStats.getStdDev();
  
  //  printf("time: %ld\tz: %d\tstd: %d\ttap: %d\r\n", uBit.systemTime(), int(val1), int(1000*std), int(1000*val/(std+g_tapScaleDenominator)));

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

    bool shouldCheckTap = g_tapCountdown > 0; //  && g_tapCountdown <= g_tapK;
    bool shouldCheckShake = g_shakeStats.getVar() > shakeGateThreshSquared;

    int zDiff = abs(sample.z - g_sampleDelay.getDelayedSample(2).z);
    if (zDiff >= tapGateThresh)
    {
        g_tapCountdown = (g_tapWindowSize/2) + g_tapK;
    }
    else if(g_tapCountdown > 0)
    {
        g_tapCountdown -= 1;
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
