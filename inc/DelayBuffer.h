#pragma once
#include "FastMath.h"
#include "RingBuffer.h"

// TODO: use ring buffer of size N+1? With delay of N, we want to get sample delayed by any amount in [0,N] (no?)

template <typename T, int N>
class DelayBuffer
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
    RingBuffer<T, N> buffer;
};

