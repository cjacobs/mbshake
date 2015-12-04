#pragma once

class eventThresholdFilter
{
public:
    eventThresholdFilter(float gestureThreshold, int eventCountThreshold) : count_(0), gestureThreshold_(gestureThreshold), eventCountThreshold_(eventCountThreshold) {};
    bool filterValue(float value)
    {
        if(value >= gestureThreshold_)
        {
            count_++;
            if (count_ >= eventCountThreshold_)
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

    void reset()
    {
        count_ = 0;
    }

private:
    int count_ = 0;
    float gestureThreshold_;
    int eventCountThreshold_;
};

