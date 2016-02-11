#pragma once

#include "DelayBuffer.h"
#include "FastMath.h"

template <typename T>
class IdentityAccessor
{
public:
    static T get_val(const T& val)
    {
        return val;
    }
};

// the "Accessor" helper classes let you transform the input type S before casting it to type T.
// Uses include extracting a single element from a vector (using e.g., the GetX accessor class)
// An Accessor has a single static function get_val() that takes a value of type S and returns a value of type T
//  (maybe we should call it a transformer...)
template <int WindowSize, int BufferSize, typename T, typename S=T, typename Accessor=IdentityAccessor<S>>
class RunningStats
{
public:
    RunningStats(DelayBuffer<S, BufferSize>& delayLine) : delayLine_(delayLine)
    {
    }

    void addSample(const S& val) // must always call this after adding sample to delay buffer
    {
        // subtract old values
        T oldVal = (T)(Accessor::get_val(delayLine_.getDelayedSample(windowSize_)));
        accumSum_ -= oldVal;
        accumSumSq_ -= (oldVal*oldVal);
        
        // add new ones
        T newVal = (T)(Accessor::get_val(val));
        accumSum_ += newVal;
        accumSumSq_ += (newVal*newVal);
    }

    T getMean()
    {
        return accumSum_ / windowSize_;
    }

    float getMeanFloat()
    {
        return (float)accumSum_ / windowSize_;
    }

    float getVar()
    {
        return float(accumSumSq_ - (accumSum_*accumSum_/windowSize_)) / windowSize_;
    }

    float getStdDev()
    {
        return 1.0f / fast_inv_sqrt(getVar());
    }

/*
    T getMax()
    {
        // For this, we have to scan the whole buffer. :(
    }
    */

private:
    const int windowSize_ = WindowSize;
    DelayBuffer<S, BufferSize>& delayLine_;

    T accumSum_ = 0;
    T accumSumSq_ = 0;
};

// convenience function to make a RunningStats with window size 1 less than the input buffer size, and of the same type
template<int BufferSize, typename T>
RunningStats<BufferSize-1, BufferSize, T, T> makeStats(DelayBuffer<T, BufferSize>& delayLine)
{
    return RunningStats<BufferSize-1, BufferSize, T>(delayLine);
}
