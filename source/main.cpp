#include "MicroBitTouchDevelop.h"
#include "vec3.h"
#include "delayBuffer.h"
#include "eventThresholdFilter.h"
#include "iirFilter.h"

// Constants
const int sampleRate = 18; // in ms
const int eventDisplayPeriod = 250; // ms

const float lowpassFilterCoeff = 0.9;
const float gravityFilterCoeff = 0.05;
const int minLenThresh = 25; //150*150;

const float shakeGestureThreshold = 1.0;
const int shakeEventCountThreshold = 3;
eventThresholdFilter shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold);

const float tapGestureThreshold = 1.7;
const int tapEventCountThreshold = 1;

const int g_tapWindowSize = 11;
const float g_tapScaleDenominator = 2.5;

const int meanBufferSize = 11;

const int dotWavelength1 = 6;
const int dotWavelength2 = 13;
const int delayBufferSize = 2*(dotWavelength2) + meanBufferSize;


// Globals
unsigned long g_prevTime = 0;
unsigned long g_turnOffDisplayTime = 0;

floatVec3 g_gravity = {0,0,0};

byteVec3 g_sampleDelayMem[delayBufferSize];
delayBuffer<byteVec3> g_sampleDelay(g_sampleDelayMem, delayBufferSize);
runningStats<long, byteVec3, GetZ<int8_t>> g_tapStats(g_tapWindowSize, g_sampleDelay);
eventThresholdFilter tapEventFilter(tapGestureThreshold, tapEventCountThreshold);

float g_meanDelay1Mem[meanBufferSize+1];
delayBuffer<float> g_meanDelay1(g_meanDelay1Mem, meanBufferSize+1);
runningStats<float> g_delayDotStats(meanBufferSize, g_meanDelay1);

//float g_meanDelay2Mem[meanBufferSize];
//delayBuffer<float> g_meanDelay2(g_meanDelay2Mem, meanBufferSize);



// Code
byteVec3 getAccelData()
{
    return byteVec3(uBit.accelerometer.getX()>>4,
                    uBit.accelerometer.getY()>>4,
                    uBit.accelerometer.getZ()>>4);
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

template <typename T>
int8_t clampByte(const T& inVal)
{
    return inVal < -127 ? -127 : inVal > 128 ? 128 : inVal;
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

    g_sampleDelay.addSample(currentSample);
    g_tapStats.addSample(currentSample);
    
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
        g_meanDelay1.addSample(0.0); // 
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
    float mean = g_delayDotStats.getMean();
    //    printf("shake: %d\r\n", int(1000*mean));
    return g_delayDotStats.getMean();
}

float getTapPrediction()
{
  float val1 = g_sampleDelay.getDelayedSample(g_tapWindowSize/2).z - g_tapStats.getMean();
  float val2 = g_sampleDelay.getDelayedSample(g_tapWindowSize/2 - 1).z - g_tapStats.getMean();
  float val = val1 - 0.25*val2;
  float std = g_tapStats.getStdDev();
  
  printf("time: %ld\tz: %d\tstd: %d\ttap: %d\r\n", uBit.systemTime(), int(val1), int(1000*std), int(1000*val/(std+g_tapScaleDenominator)));

  return val / (std + g_tapScaleDenominator);
}

enum MicroBitAccelerometerEvents
    {
        MICROBIT_ACCELEROMETER_SHAKE = 100,
        MICROBIT_ACCELEROMETER_TAP = 101,
    };


void onShake(MicroBitEvent)
{
  uBit.display.print('S');
    // TODO: set a timer to turn off?
}

void onTap(MicroBitEvent)
{
    uBit.display.print('T');
}

int detectGesture()
{
    uBit.accelerometer.update();
    byteVec3 sample = getAccelData();
    
    processSample(sample);

    float tapPredVal = getTapPrediction();
    bool foundTap = tapEventFilter.filterValue(tapPredVal);
    if(foundTap)
    {
      printf("########\r\n");
        shakeEventFilter.reset();
        return MICROBIT_ACCELEROMETER_TAP;
    }

    float shakePredVal = getShakePrediction();
    bool foundShake = shakeEventFilter.filterValue(shakePredVal);
    if(foundShake)
    {
        tapEventFilter.reset();
        return MICROBIT_ACCELEROMETER_SHAKE;
    }

    
    return 0;
}

/*
void printStuff()
{
    byteVec3 currentSample = byteVec3 {sample.x-g_gravity.x, sample.y-g_gravity.y, sample.z-g_gravity.z};
    int len = (int)sqrt(currentSample.x*currentSample.x + currentSample.y*currentSample.y + currentSample.z*currentSample.z);
    printf("%ld\t--\t%d - \t%d\t%d\t%d\t: %d\r\n", (time-g_prevTime), (int)(val*1000), currentSample.x, currentSample.y, currentSample.z, len);
}
*/

void accelerometer_poll()
{
    while(true)
    {
        unsigned long time = uBit.systemTime();
        // If enough time has elapsed or the timer rolls over, do something
        if ((time-g_prevTime) >= sampleRate || time < g_prevTime) 
        {
            if (time > g_turnOffDisplayTime)
            {
                uBit.display.clear();
            }

            int detectedGesture = detectGesture();
            if(detectedGesture != 0)
            {
                g_turnOffDisplayTime = time + eventDisplayPeriod;

                // has the side-effect of dispatching the event onto the message bus
                MicroBitEvent(MICROBIT_ID_ACCELEROMETER, detectedGesture);
            }
            g_prevTime = time;
        }
        uBit.sleep(1);
    }
}

void app_main()
{
    // init gravity
    uBit.accelerometer.update();
    g_gravity = floatVec3(getAccelData());
    
    // create background worker that polls for shake events
    create_fiber(accelerometer_poll);

    // ... and listen for them
    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_SHAKE, onShake);
    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_TAP, onTap);
}
