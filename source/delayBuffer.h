#pragma once
#include "fastmath.h"

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

private:
    T* buffer;
    int bufLen;
    int currentPos;
};

