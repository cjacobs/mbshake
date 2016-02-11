#pragma once

#include "DelayBuffer.h"
#include "RunningStats.h"
#include "EventThresholdFilter.h"
#include "Vector3.h"
#include "IirFilter.h"

enum MicroBitAccelerometerEvents
    {
        MICROBIT_ACCELEROMETER_SHAKE = 100,
        MICROBIT_ACCELEROMETER_TAP = 101,
    };

class GestureDetector
{
public:
    GestureDetector();
    void init();

    void systemTick();
    int getCurrentGesture();

    void togglePrinting();
    void toggleAlg();

private:
    void processSample(byteVector3 sample);
    template<typename MeanDelayType, typename MeanStatsType>
    void processDotFeature(const byteVector3& currentSample, int dotWavelength, MeanDelayType& meanDelay, MeanStatsType& delayDotStats);

    float getShakePrediction();
    float getTapPrediction();
    int detectGesture(); // needs to be called at 50hz (for now)

    // Compile-time constants (used as template parameters)
    static constexpr int dotWavelength1 = 4;
    static constexpr int dotWavelength2 = 5;
    static constexpr int dotWavelength3 = 6;
    static constexpr int dotWavelength4 = 8;

    static constexpr int dotMeanWindow1 = dotWavelength1; // / 2;
    static constexpr int dotMeanWindow2 = dotWavelength2; // / 2;
    static constexpr int dotMeanWindow3 = dotWavelength3; // / 2;
    static constexpr int dotMeanWindow4 = dotWavelength4; // / 2;
    
    static constexpr int shakeStatsBufferSize = 10;
    static constexpr int delayBufferSize = 2*(dotWavelength4) + shakeStatsBufferSize;
    static constexpr int tapK = 2;

    static constexpr int tapLargeWindowSize = 8; //11; // maybe too big?
    static constexpr int tapImpulseWindowSize = 2;

    // Data
    int8_t state;

    floatVector3 gravity;
    floatVector3 filteredSample; // used when lowpass filtering

    SimpleIirFilter<floatVector3, float> gravityFilter;

    // Global delay buffer for filtered, gravity-subtracted accel input
    DelayBuffer<byteVector3, delayBufferSize> sampleDelayBuffer;
    RunningStats<tapLargeWindowSize, delayBufferSize, long, byteVector3, GetZ<int8_t>> tapLargeWindowStats;
    
    // windowed statistics for detecting high-Z-energy area during tap
    RunningStats<tapImpulseWindowSize, delayBufferSize, long, byteVector3, GetZ<int8_t>> tapImpulseWindowStats;
    
    // Shake gesture stats
    RunningStats<shakeStatsBufferSize, delayBufferSize, float, byteVector3, GetMagSq<int8_t, float>> shakeThreshStats;
    
    // TODO: quantize these to shorts or something
    DelayBuffer<float, dotMeanWindow1 + 1> dotDelayBuffer1;
    RunningStats<dotMeanWindow1, dotMeanWindow1+1, float> dot1Stats;
    
    DelayBuffer<float, dotMeanWindow2 + 1> dotDelayBuffer2;
    RunningStats<dotMeanWindow2, dotMeanWindow2+1, float> dot2Stats;
    
    DelayBuffer<float, dotMeanWindow3 + 1> dotDelayBuffer3;
    RunningStats<dotMeanWindow3, dotMeanWindow3+1, float> dot3Stats;
    
    DelayBuffer<float, dotMeanWindow4 + 1> dotDelayBuffer4;
    RunningStats<dotMeanWindow4, dotMeanWindow4+1, float> dot4Stats;
    
    DelayBuffer<float, tapK+1> quietVarDelay;

    EventThresholdFilter<float> shakeEventFilter;
    EventThresholdFilter<float> tapEventFilter;

    // timing stuff
    unsigned long prevTime = 0;

    int tapCountdown1 = 0;
    float prevQuietVar = 0;
    
    // diagnostic stuff
    bool isPrinting = true;
    bool printShake = true;
    bool allowSlowGesture = false;

    byteVector3 lastRawSample;
    byteVector3 lastFilteredSample;
};

