#pragma once

#include <array>

template <typename T, int N>
class ring_buffer
{
public:
    ring_buffer();
    T operator[](int index) const;
    void push_back(const T& val);
    size_t size() const;

private:    
    std::array<T, N> arr_;
    int curr_pos_ = 0;
};


template <typename T, int N>
ring_buffer<T,N>::ring_buffer()
{
}

template <typename T, int N>
T ring_buffer<T,N>::operator[](int index) const
{
    return arr_[index+arr_.size()]%arr_.size();
}

template <typename T, int N>
void ring_buffer<T,N>::push_back(const T& val)
{
    curr_pos_ = (curr_pos_+1)%arr_.size();
    arr_[curr_pos_] = val;
}

template <typename T, int N>
size_t ring_buffer<T,N>::size() const
{
    return arr_.size();
}
