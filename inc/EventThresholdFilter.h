#pragma once

template <typename T>
class EventThresholdFilter
{
public:
    EventThresholdFilter(T gestureThreshold, int eventCountThreshold, int lowThreshold) : count_(0), gestureThreshold_(gestureThreshold), eventCountThreshold_(eventCountThreshold), lowThreshold_(lowThreshold) {};

    bool filterValue(T value)
    {
        if(value >= gestureThreshold_)
        {
            count_++;
            if (count_ >= eventCountThreshold_ + lowThreshold_)
            {
                count_ = eventCountThreshold_ + lowThreshold_; // avoid overflow
            }
        }
        else
        {
            count_--;
            if (count_ < eventCountThreshold_)
            {
                count_ = 0;
            }
        }

        return count_ >= eventCountThreshold_;
    }

    bool currentValue()
    {
        return count_ >= eventCountThreshold_;
    }

    void reset()
    {
        count_ = 0;
    }

private:
    int count_ = 0;
    T gestureThreshold_;
    int eventCountThreshold_;
    int lowThreshold_;
};

