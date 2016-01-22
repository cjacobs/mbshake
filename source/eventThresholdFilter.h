#pragma once

template <typename T>
class eventThresholdFilter
{
public:
    eventThresholdFilter(T gestureThreshold, int eventCountThreshold) : count_(0), gestureThreshold_(gestureThreshold), eventCountThreshold_(eventCountThreshold) {};
    bool filterValue(T value)
    {
        if(value >= gestureThreshold_)
        {
            count_++;
            if (count_ >= eventCountThreshold_)
            {
                count_ = eventCountThreshold_; // avoid overflow
                return true;
            }
        }
        else
        {
            count_ = 0;
        }
        return false;
    }

    void reset()
    {
        count_ = 0;
    }

private:
    int count_ = 0;
    T gestureThreshold_;
    int eventCountThreshold_;
};

