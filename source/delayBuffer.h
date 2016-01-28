#pragma once
#include "fastmath.h"
#include "ringBuffer.h"

// TODO: use ring buffer of size N+1? With delay of N, we want to get sample delayed by any amount in [0,N]

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

