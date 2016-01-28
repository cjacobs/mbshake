#pragma once
#include "fastmath.h"
#include "ringBuffer.h"

template <typename T, int N>
class delayBuffer
{
public:
    void addSample(T val)
    {
        buffer.push_back(val);
    }

    T getDelayedSample(int delay)
    {
        return buffer[-delay];
    }

private:
    ringBuffer<T, N> buffer;
};

