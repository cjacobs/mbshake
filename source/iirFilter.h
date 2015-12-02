#pragma once

#include <array>
#include <algorithm>

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

template <int M, int N>
class iir_filter
{
public:
    iir_filter(const std::array<float, M>&, const std::array<float, N>&);
    float filterSample(float x);

private:
    std::array<float, M> a_; // feedback (y) coefficients
    std::array<float, N-1> b_; // feed-forward (x) coefficients
    float b0_; // first b coefficient
    ring_buffer<float, M> y_prev_;
    ring_buffer<float, N-1> x_prev_;

    // y[t] = x[t]*b0 + x[t-1]*b[0] + x[t-2]*b[1] + ... +
    //                  y[t-1]*a[0] + y[t-2]*b[1] ...
};

template <int M, int N>
iir_filter<M,N>::iir_filter(const std::array<float, M>& a, const std::array<float, N>& b)
{
    a_ = a;
    b0_ = b[0];
    std::copy(b.begin()+1, b.end(), b_.begin());
}

template <int M, int N>
float iir_filter<M,N>::filterSample(float x)
{
    float y = b0_*x;
    for(int index = 0; index < b_.size()-1; index++)
    {
        y += b_[index]*x_prev_[-index];
    }
    for(int index = 0; index < a_.size()-1; index++)
    {
        y += a_[index]*y_prev_[-index];
    }

    x_prev_.push_back(x);
    y_prev_.push_back(y);

    return y;
}
