#include "MicroBitTouchDevelop.h"

// Types
template <typename T>
struct vec3
{
    T x;
    T y;
    T z;

    vec3<T>() : x(0), y(0), z(0) {}
    vec3<T>(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename S>
    explicit vec3<T>(const vec3<S>& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }
};

typedef vec3<int8_t> byteVec3;
typedef vec3<short> shortVec3;
typedef vec3<float> floatVec3;

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

// Constants
const int sampleRate = 6; // as fast as possible (ends up being every 6 ms)
const int eventDisplayPeriod = 250; // ms

const float gravityFilterCoeff = 0.01; //0.0001;
const float gestureThreshold = 0.65;
const int eventCountThreshold = 3;
const int minLenThresh = 25; //150*150;

const int meanBufferSize = 16;

const int dotWavelength1 = 20;
const int dotWavelength2 = 2*dotWavelength1;
const int delayBufferSize = dotWavelength2 + meanBufferSize;


// Globals
floatVec3 gravity = {0,0,0};
floatVec3 currentSample = {0,0,0};

byteVec3 sampleDelayMem[delayBufferSize];
delayBuffer<byteVec3> sampleDelay(sampleDelayMem, delayBufferSize);


static float meanDelay1Mem[meanBufferSize];
float meanDelay2Mem[meanBufferSize];
delayBuffer<float> meanDelay1(meanDelay1Mem, meanBufferSize);
delayBuffer<float> meanDelay2(meanDelay2Mem, meanBufferSize);

// Code
byteVec3 getAccelData()
{
    return byteVec3(uBit.accelerometer.getX()>>4,
                    uBit.accelerometer.getY()>>4,
                    uBit.accelerometer.getZ()>>4);
}

void filterVec(const floatVec3& vec, floatVec3& prevVec, float alpha)
{
    prevVec.x += alpha*(vec.x - prevVec.x);
    prevVec.y += alpha*(vec.y - prevVec.y);
    prevVec.z += alpha*(vec.z - prevVec.z);
}

template<typename T>
float dot(const vec3<T>& a, const vec3<T>& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

/*
float dotNorm(const floatVec3& a, const floatVec3& b, float minLenThresh)
{
    float val = dot(a,b);
    float aLenSq = dot(a,a);
    float bLenSq = dot(b,b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    return val / (sqrt(aLenSq)*sqrt(bLenSq));
}
*/
template <typename T>
float dotNorm(const vec3<T>& a, const vec3<T>& b, int minLenThresh)
{
    float val = dot(a,b);
    float aLenSq = dot(a,a);
    float bLenSq = dot(b,b);

    if (aLenSq < minLenThresh || bLenSq < minLenThresh)
    {
        return 0;
    }

    return val / (sqrt(aLenSq)*sqrt(bLenSq));
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

float processSample(byteVec3 sample)
{
    filterVec(floatVec3(sample), gravity, gravityFilterCoeff);
    byteVec3 currentSample = byteVec3 {sample.x-gravity.x, sample.y-gravity.y, sample.z-gravity.z};
    sampleDelay.addSample(currentSample);

    // get mean of dot with past
    /*
    float val = 0;
    for(int index = 0; index < meanBufferSize; index++)
    {
        float dot1 = dotNorm(sampleDelay.getDelayedSample(index), sampleDelay.getDelayedSample(index+dotWavelength1), minLenThresh);
        float dot2 = dotNorm(sampleDelay.getDelayedSample(index), sampleDelay.getDelayedSample(index+dotWavelength2), minLenThresh);
        val += (dot2-dot1);
    }
    val /= meanBufferSize;
    return val;
    */

    // now add val to mean buffer
    float dot1 = dotNorm(currentSample, sampleDelay.getDelayedSample(dotWavelength1), minLenThresh);
    float dot2 = dotNorm(currentSample, sampleDelay.getDelayedSample(dotWavelength2), minLenThresh);
    meanDelay1.addSample(dot1);
    meanDelay2.addSample(dot2);
    
    float sum1 = sum(meanDelay1Mem, meanBufferSize);
    float sum2 = sum(meanDelay2Mem, meanBufferSize);
    return (sum2-sum1) / meanBufferSize;

}

static int count = 0;
bool filterDetector(float val, float gestureThreshold, int eventCountThreshold)
{
    if(val > gestureThreshold)
    {
        count++;
        if (count > eventCountThreshold)
        {
            return true;
        }
    }
    else
    {
        count = 0;
    }
    return false;
}

void app_main()
{
    unsigned long prevTime = 0;
    unsigned long turnOffDisplayTime = 0;
    uBit.accelerometer.update();
    gravity = floatVec3(getAccelData()); // init gravity

    while(true)
    {
        unsigned long time = uBit.systemTime();
        // if enough time has elapsed or the timer rolls over, do something
        if ((time-prevTime) >= sampleRate || time < prevTime) 
        {
            if (time > turnOffDisplayTime)
            {
                uBit.display.clear();
            }

            uBit.accelerometer.update();
            auto sample = getAccelData();
            float val = processSample(sample);
            bool gotShakeEvent = filterDetector(val, gestureThreshold, eventCountThreshold);
            if(gotShakeEvent)
            {
                uBit.display.clear();
                uBit.display.print('x');
                turnOffDisplayTime = time + eventDisplayPeriod;
                printf("#### ");
            }
            else
            {
                printf("     ");
            }

            byteVec3 currentSample = byteVec3 {sample.x-gravity.x, sample.y-gravity.y, sample.z-gravity.z};
            int len = (int)sqrt(currentSample.x*currentSample.x + currentSample.y*currentSample.y + currentSample.z*currentSample.z);
            printf("%ld\t--\t%d - \t%d\t%d\t%d\t: %d\r\n", (time-prevTime), (int)(val*1000), currentSample.x, currentSample.y, currentSample.z, len);

            prevTime = time;
        }
    }
}
