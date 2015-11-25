#pragma once

class eventThresholdFilter
{
public:
    eventThresholdFilter(float gestureThreshold, int eventCountThreshold) : gestureThreshold_(gestureThreshold), eventCountThreshold_(eventCountThreshold), count_(0) {};
    bool filterValue(float value)
    {
        if(value > gestureThreshold_)
        {
            count_++;
            if (count_ > eventCountThreshold_)
            {
                return true;
            }
        }
        else
        {
            count_ = 0;
        }
        return false;
    }

private:
    int count_ = 0;
    float gestureThreshold_;
    int eventCountThreshold_;
};

