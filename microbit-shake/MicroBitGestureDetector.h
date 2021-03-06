#pragma once

#include "DelayBuffer.h"
#include "RunningStats.h"
#include "EventThresholdFilter.h"
#include "Vector3.h"
#include "IirFilter.h"
#include "FixedPt.h"


// #defines for optional parts
#define USE_SHAKE_GATE 0

//using filteredComponent_t = float;
using filteredComponent_t = fixed_9_7;
using filteredSample_t = Vector3<filteredComponent_t>;

#define FIXED_MATH 1
#if FIXED_MATH
using predictionValue_t = fixed_9_7;
#else
using predictionValue_t = float;
#endif

// using filterCoeff_t = float;
using filterCoeff_t = fixed_2_14;

enum MicroBitAccelerometerEvents
    {
        MICROBIT_ACCELEROMETER_SHAKE = 100,
        MICROBIT_ACCELEROMETER_TAP = 101,
    };

class MicroBitGestureDetector
{
public:
    MicroBitGestureDetector();
    void init();

    void systemTick();
    int getCurrentGesture();
    bool isShaking();

    void togglePrinting();
    void toggleAlg();

private:
    void processSample(byteVector3 sample);
    template<typename MeanDelayType, typename MeanStatsType>
    void processDotFeature(const byteVector3& currentSample, int dotWavelength, MeanDelayType& meanDelay, MeanStatsType& delayDotStats);

    predictionValue_t getShakePrediction();
    float getTapPrediction();
    int detectGesture(); // needs to be called at 50hz (for now)

    // Compile-time constants (used as template parameters)
    static constexpr int dotWavelength2 = 5;
    static constexpr int dotWavelength4 = 8;

    static constexpr int dotMeanWindow2 = dotWavelength2; // / 2;
    static constexpr int dotMeanWindow4 = dotWavelength4; // / 2;
    
    static constexpr int shakeStatsBufferSize = 4;
    static constexpr int delayBufferSize = 2*(dotWavelength4) + shakeStatsBufferSize;
    static constexpr int tapK = 2;

    static constexpr int tapLargeWindowSize = 8; //11; // maybe too big?
    static constexpr int tapImpulseWindowSize = 2;

    // Data
    int8_t state;

    filteredSample_t gravity;
    filteredSample_t filteredSample; // used when lowpass filtering
    
    SimpleIirFilter<filteredSample_t, filterCoeff_t> gravityFilter;

    // Global delay buffer for filtered, gravity-subtracted accel input
    DelayBuffer<byteVector3, delayBufferSize> sampleDelayBuffer;
    RunningStats<tapLargeWindowSize, delayBufferSize, long, byteVector3, GetZ<int8_t>> tapLargeWindowStats;
    
    // windowed statistics for detecting high-Z-energy area during tap
    RunningStats<tapImpulseWindowSize, delayBufferSize, long, byteVector3, GetZ<int8_t>> tapImpulseWindowStats;
    
    // Shake gesture stats
#if USE_SHAKE_GATE
    RunningStats<shakeStatsBufferSize, delayBufferSize, float, byteVector3, GetMagSq<int8_t, float>> shakeThreshStats;
#endif
    
    // TODO: these can easily be fixed-pt (but check range of dotNorm function)
    // TODO: quantize these to shorts or something
    DelayBuffer<predictionValue_t, dotMeanWindow2 + 1> dotDelayBuffer2;
    RunningMean<dotMeanWindow2, dotMeanWindow2+1, predictionValue_t> dot2Stats;
    
    DelayBuffer<predictionValue_t, dotMeanWindow4 + 1> dotDelayBuffer4;
    RunningMean<dotMeanWindow4, dotMeanWindow4+1, predictionValue_t> dot4Stats;
    
    DelayBuffer<float, tapK+1> quietVarDelay;

    // TODO: fixed-pt
    EventThresholdFilter<predictionValue_t> shakeEventFilter;
    EventThresholdFilter<float> tapEventFilter;

    // timing stuff
    unsigned long prevTime = 0;

    int tapCountdown1 = 0;

    // diagnostic stuff
    bool isPrinting = false;
    bool printShake = true;
    bool allowSlowGesture = false;

    byteVector3 lastRawSample;
    byteVector3 lastFilteredSample;
};

