#include "MicroBitTouchDevelop.h"
#include "vec3.h"
#include "iir_filter.h"

// Types

template <typename T>
class delayBuffer
{
public:
    delayBuffer(T* buffer, int bufLen) : buffer(buffer), bufLen(bufLen)
    {
        currentPos = 0;
        for(int index = 0; index < bufLen; index++)
        {
            buffer[index] = T();
        }
    }

    void addSample(T val)
    {
        currentPos = (currentPos+1) % bufLen;
        buffer[currentPos] = val;
    }

    T getDelayedSample(int delay)
    {
        // need to ensure offset is positive
        return buffer[(currentPos+2*bufLen-delay)%bufLen];
    }

public:
    T* buffer;
    int bufLen;
    int currentPos;
};


class eventThresholdFilter
{
public:
    eventThresholdFilter(float gestureThreshold, int eventCountThreshold) : gestureThreshold_(gestureThreshold), eventCountThreshold_(eventCountThreshold), count_(0) {};
    bool filterValue(float value)
    {
        if(value > gestureThreshold_)
        {
            count_++;
            if (count_ > eventCountThreshold_)
            {
                return true;
            }
        }
        else
        {
            count_ = 0;
        }
        return false;
    }

private:
    int count_ = 0;
    float gestureThreshold_;
    int eventCountThreshold_;
};


// Constants
const int sampleRate = 6; // as fast as possible (ends up being every 6 ms)
const int eventDisplayPeriod = 250; // ms

const float gravityFilterCoeff = 0.0001; //0.0001;
const int minLenThresh = 25; //150*150;

const float shakeGestureThreshold = 0.65;
const int shakeEventCountThreshold = 3;

eventThresholdFilter shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold);

const float tapGestureThreshold = 0.65;
const int tapEventCountThreshold = 3;

const bool useMeanBuffer = true;
const int meanBufferSize = 16;

const int dotWavelength1 = 20;
const int dotWavelength2 = 25;
const int delayBufferSize = 2*(dotWavelength2) + meanBufferSize;


// Globals
unsigned long g_prevTime = 0;
unsigned long g_turnOffDisplayTime = 0;

floatVec3 g_gravity = {0,0,0};

byteVec3 g_sampleDelayMem[delayBufferSize];
delayBuffer<byteVec3> g_sampleDelay(g_sampleDelayMem, delayBufferSize);

float g_meanDelay1Mem[meanBufferSize];
float g_meanDelay2Mem[meanBufferSize];
delayBuffer<float> g_meanDelay1(g_meanDelay1Mem, meanBufferSize);
delayBuffer<float> g_meanDelay2(g_meanDelay2Mem, meanBufferSize);

static int g_currentShakeOutputCount = 0;

// Code
byteVec3 getAccelData()
{
    return byteVec3(uBit.accelerometer.getX()>>4,
                    uBit.accelerometer.getY()>>4,
                    uBit.accelerometer.getZ()>>4);
}

// TODO: use iir_filter object here
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
    filterVec(floatVec3(sample), g_gravity, gravityFilterCoeff);
    byteVec3 currentSample = byteVec3 {sample.x-g_gravity.x, sample.y-g_gravity.y, sample.z-g_gravity.z};
    g_sampleDelay.addSample(currentSample);
    
    if(useMeanBuffer)
    {
        // now add val to mean buffer
        float dot1a = dotNorm(currentSample, g_sampleDelay.getDelayedSample(dotWavelength1), minLenThresh);
        float dot1b = dotNorm(currentSample, g_sampleDelay.getDelayedSample(2*dotWavelength1), minLenThresh);
        if(dot1a < 0 && dot1b > 0)
            g_meanDelay1.addSample(dot1b-dot1a);
        else
            g_meanDelay1.addSample(0.0); // ?
    }
}

float getShakePrediction()
{    
    // get mean of dot with past
    if(!useMeanBuffer)
    {
        float val = 0;
        for(int index = 0; index < meanBufferSize; index++)
        {
            float dot1a = dotNorm(g_sampleDelay.getDelayedSample(index), g_sampleDelay.getDelayedSample(index+dotWavelength1), minLenThresh);
            float dot1b = dotNorm(g_sampleDelay.getDelayedSample(index), g_sampleDelay.getDelayedSample(index+2*dotWavelength1), minLenThresh);
            val += (dot1b-dot1a);
        }
        val /= meanBufferSize;
        return val;
    }
    else
    {
        return sum(g_meanDelay1Mem, meanBufferSize) / meanBufferSize;
    }
}

float getTapPrediction()
{
    return 0.0;
}

enum MicroBitAccelerometerEvents
    {
        MICROBIT_ACCELEROMETER_SHAKE = 100,
        MICROBIT_ACCELEROMETER_TAP = 101,
    };


void onShake(MicroBitEvent evt)
{
    uBit.display.print('#');
    // TODO: set a timer to turn off?
}

void onTap(MicroBitEvent evt)
{
    uBit.display.print('T');
}

int detectGesture()
{
    uBit.accelerometer.update();
    byteVec3 sample = getAccelData();

    
    processSample(sample);
    float shakePredVal = getShakePrediction();
    bool foundShake = shakeEventFilter.filterValue(shakePredVal);
    if(foundShake)
    {
        return MICROBIT_ACCELEROMETER_SHAKE;
    }

    float tapPredVal = getTapPrediction();
    //    bool foundTap = filterDetectorOutput(tapPredVal, tapGestureThreshold, tapEventCountThreshold);
    //    if(foundTap)
    //    {
    //        return MICROBIT_ACCELEROMETER_TAP;
    //    }

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
    //    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_TAP, onTap);
}
