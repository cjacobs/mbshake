#pragma once

#include "ringBuffer.h"
#include <array>
#include <algorithm>

// M == len(a)
// N == len(b)
// a == coeffs on y_prev
// b == coeffs on x_prev
// pass in 'b' coeffs (the FIR coeffs) first. We're always assuming an initial 'a' coeff of 1.0, though
// sum(a[i]*y[t-i]) = sum(b[i]*x[t-i])

template <typename T, int N, int M>
class iir_filter
{
public:
    iir_filter(const std::array<float, N>& b, const std::array<float, M>& a); // pass in a[1:], assuming a[0] == 1.0
    T filterSample(const T& x);

private:
    std::array<float, M> a_; // feedback (y) coefficients
    std::array<float, (N>0?N-1:0)> b_; // feed-forward (x) coefficients
    float b0_; // first b coefficient (separate just to make code nicer in filterSample

    ring_buffer<T, M> y_prev_;
    ring_buffer<T, (N>0?N-1:0)> x_prev_;

    // y[t] = x[t]*b0 + x[t-1]*b[0] + x[t-2]*b[1] + ... 
    //                - y[t-1]*a[0] - y[t-2]*b[1] ...
};

template <typename T, int N, int M>
iir_filter<T,N,M>::iir_filter(const std::array<float, N>& b, const std::array<float, M>& a)
{
    a_ = a;
    b0_ = b[0];
    std::copy(b.begin()+1, b.end(), b_.begin());
}

template <typename T, int N, int M>
T iir_filter<T,N,M>::filterSample(const T& x)
{
    T y = b0_*x;
    for(int index = 0; index < b_.size()-1; index++)
    {
        y += b_[index]*x_prev_[-index];
    }
    for(int index = 0; index < a_.size(); index++)
    {
        y -= a_[index]*y_prev_[-index];
    }

    if(N > 1) x_prev_.push_back(x);
    if(M > 0) y_prev_.push_back(y);

    return y;
}
