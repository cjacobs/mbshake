#pragma once

#include "delayBuffer.h"
#include "fastmath.h"

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
template <typename T, typename S=T, typename Accessor=IdentityAccessor<S>>
class runningStats
{
public:
    runningStats(int windowSize, delayBuffer<S>& delayLine) : windowSize_(windowSize), delayLine_(delayLine)
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
        return (accumSumSq_ - (accumSum_*accumSum_/windowSize_)) / windowSize_;
    }

    float getStdDev()
    {
        return 1.0f / fast_inv_sqrt(getVar());
    }

private:
    int windowSize_ = 0;
    delayBuffer<S>& delayLine_;

    T accumSum_ = 0;
    T accumSumSq_ = 0;
};
