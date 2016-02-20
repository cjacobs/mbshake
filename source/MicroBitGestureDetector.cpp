#include "Vector3.h"
#include "DelayBuffer.h"
#include "RunningStats.h"
#include "EventThresholdFilter.h"
#include "IirFilter.h"
#include "MicroBitAccess.h"
#include "FixedPt.h"
#include "MicroBitGestureDetector.h"

#include <cmath>
#include <cstdlib>

#define QUANTIZE_SAMPLE 0


using std::abs;
using std::max;

const int sampleRate = 18; // in ms

const filterCoeff_t gravityFilterCoeff = filterCoeff_t(1/32.0);

const float minLenThresh = 1; 

const float shakeGestureThreshold = 0.4f;
const int shakeEventCountThreshold = 6;
const int shakeEventCountLowThreshold = 3;

#if USE_SHAKE_GATE
const float shakeGateThreshSquared = 40000; //4000000.0f;
#endif

// Tap stuff
const float tapGestureThreshold = 200.0f; // TODO: this can be an int
const int tapEventCountThreshold = 1;

const float tapScaleDenominator = 2.5f;
const float tapGateThresh1 = 25.0f; // variance of preceeding windown should be less than this



int getRectArea(int w, int h)
{
    return w*h;
}




// TODO:
// * Maybe modulate the shake output slightly by the amount of energy? --- soft shakes
//     return a very large prediction value --- often higher than a strong shake
// * Maybe tune shake frequency by energy? hard shakes are somewhat slower (are they?)
// * Maybe use max over some window instead of mean for shake pred value? (though this
//     risks making transitory spikes last longer and be harder to filter out

// TODO: still doesn't detect taps if device is anchored to a solid
// object (like atable). Then, the var over the big window is
// [0,0,0,0,0... ~12, ...]  Maybe check if var over an even bigger
// window is exactly(ish) 0, and lower the threshold even more if so?

MicroBitGestureDetector::MicroBitGestureDetector() : gravityFilter(gravityFilterCoeff),
                                     tapLargeWindowStats(sampleDelayBuffer),
                                     tapImpulseWindowStats(sampleDelayBuffer),
#if USE_SHAKE_GATE
                                     shakeThreshStats(sampleDelayBuffer),
#endif
                                     dot1Stats(dotDelayBuffer1),
                                     dot2Stats(dotDelayBuffer2),
                                     dot3Stats(dotDelayBuffer3),
                                     dot4Stats(dotDelayBuffer4),
    shakeEventFilter(shakeGestureThreshold, shakeEventCountThreshold, shakeEventCountLowThreshold),
    tapEventFilter(tapGestureThreshold, tapEventCountThreshold, 0)
{
    init(); // ?
}

void MicroBitGestureDetector::init()
{
    // init gravity
    updateAccelerometer();
    auto sample = getAccelData();
    filteredSample_t initFilterSample = filteredSample_t(sample);
    gravity = initFilterSample;
    filteredSample = initFilterSample;
    gravityFilter.init(initFilterSample);
}

#if QUANTIZE_SAMPLE
byteVector3 quantizeSample(const byteVector3& b, int factor)
{
    // TODO: round appropriately, be more efficient
    return byteVector3(b / float(factor))*factor;
}
#endif

// For some reason, this kills the micro:bit for a while
template<typename MeanDelayType, typename MeanStatsType>
void MicroBitGestureDetector::processDotFeature(const byteVector3& currentSample, int dotWavelength, MeanDelayType& meanDelay, MeanStatsType& delayDotStats)
{
#if QUANTIZE_SAMPLE    
    // TODO: investigate if this really helps like it appears to do in the python version
    int quantRate = 16;
    byteVector3 quantizedCurrentSample = quantizeSample(currentSample, quantRate);

    float dot1a = dotNorm(quantizedCurrentSample, quantizeSample(sampleDelayBuffer.getDelayedSample(dotWavelength), quantRate), minLenThresh);
    float dot1b = dotNorm(quantizedCurrentSample, quantizeSample(sampleDelayBuffer.getDelayedSample(2 * dotWavelength), quantRate), minLenThresh);
#else
    // TODO: implement fixed-pt versions of dotNorm
    float dot1a = dotNorm(currentSample, sampleDelayBuffer.getDelayedSample(dotWavelength), minLenThresh);
    float dot1b = dotNorm(currentSample, sampleDelayBuffer.getDelayedSample(2 * dotWavelength), minLenThresh);
#endif

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

void MicroBitGestureDetector::processSample(byteVector3 sample)
{
    lastRawSample = sample;
    gravity = gravityFilter.filterSample(filteredSample_t(sample));
    
    byteVector3 currentSample = byteVector3 { clampByte((int)sample.x-(int)gravity.x),
                                              clampByte((int)sample.y-(int)gravity.y),
                                              clampByte((int)sample.z-(int)gravity.z) };
    lastFilteredSample = currentSample;

    // TODO: maybe we should somehow associate the stats objects with the delay lines
    //       then we won't have to remember to add the stats.addSample() lines
    sampleDelayBuffer.addSample(currentSample); 
    tapLargeWindowStats.addSample(currentSample);
    tapImpulseWindowStats.addSample(currentSample);
#if USE_SHAKE_GATE
    shakeThreshStats.addSample(currentSample);
#endif
    
    // now add val to mean buffer
    processDotFeature(currentSample, dotWavelength2, dotDelayBuffer2, dot2Stats);
    if(allowSlowGesture)
    {
        processDotFeature(currentSample, dotWavelength4, dotDelayBuffer4, dot4Stats);
    }
}

predictionValue_t MicroBitGestureDetector::getShakePrediction()
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

predictionValue_t MicroBitGestureDetector::getTapPrediction()
{
    // If previous quiet window was very very quiet (e.g., 0), then
    // increase output (when micro:bit is sitting on table, tap
    // amplitude is diminished)

    float scale = fast_inv_sqrt(1.0 + quietVarDelay.getDelayedSample(tapK)); 
    return tapImpulseWindowStats.getVar() * scale;
}

void MicroBitGestureDetector::togglePrinting()
{
    isPrinting = !isPrinting;
}

void MicroBitGestureDetector::toggleAlg()
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

void MicroBitGestureDetector::systemTick()
{
    unsigned long time = systemTime();

    // If enough time has elapsed or the timer rolls over, do something
    if ((time-prevTime) >= sampleRate || time < prevTime) 
    {
        prevTime = time;
        state = detectGesture();
    }
}

int MicroBitGestureDetector::detectGesture()
{
    updateAccelerometer();
    byteVector3 sample = getAccelData();
    
    bool shouldCheckTap = tapCountdown1 > 0;
#if USE_SHAKE_GATE
    bool shouldCheckShake = shakeThreshStats.getVar() > shakeGateThreshSquared;
#else
    const bool shouldCheckShake = true;
#endif

    // criterion 1: look for N samples worth of quiet
    auto quietVariance = tapLargeWindowStats.getVar();
    quietVarDelay.addSample(quietVariance);

    if (quietVariance <= tapGateThresh1)
    {
        tapCountdown1 = tapK;
    }
    else if(tapCountdown1 > 0)
    {
        tapCountdown1 -= 1;
    }

    processSample(sample);

    float diagnosticVal = 0;
    if(isPrinting) diagnosticVal = getShakePrediction();

    //    auto& printedSample = lastRawSample;
    auto& outputSample = lastFilteredSample;

    if(shouldCheckTap)
    {
        float tapPredVal = getTapPrediction();
        bool foundTap = tapEventFilter.filterValue(tapPredVal);
        if(foundTap)
        {
            shakeEventFilter.reset();
            if(isPrinting)
            {
                serialPrintLn("Tap\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", outputSample, "\t", diagnosticVal);
            }
            return MICROBIT_ACCELEROMETER_TAP;
        }
    }

    if(shouldCheckShake)
    {
        auto shakePredVal = getShakePrediction();
        bool foundShake = shakeEventFilter.filterValue(shakePredVal);
        if(foundShake)
        {
            tapEventFilter.reset();
            if(isPrinting)
            {
                serialPrintLn("Shake\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", outputSample, "\t", diagnosticVal);
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
        serialPrintLn("\t", systemTime(), "\t", buttonA(), "\t", buttonB(), "\t", outputSample, "\t", diagnosticVal);
    }

    return 0; // none
}

int MicroBitGestureDetector::getCurrentGesture()
{
    return state;
}
