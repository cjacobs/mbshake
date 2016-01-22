#pragma once
#include "fastmath.h"

// This is just a ring buffer... why not use it?
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
        int index = (currentPos - delay) % bufLen;
        if (index < 0) index += bufLen; // need to ensure offset is positive
        return buffer[index];
    }

private:
    T* buffer;
    int bufLen;
    int currentPos;
};

