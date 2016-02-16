#pragma once

#include "RingBuffer.h"
#include "Vector3.h"
#include "FastMath.h"

#include <array>
#include <algorithm>
#include <cmath>

using std::abs;

// Simple 1-tap IIR filter with a = {-alpha} and b = {1-alpha}
inline void filterVec(const floatVector3& vec, floatVector3& prevVec, float alpha)
{
    prevVec.x += alpha*(vec.x - prevVec.x);
    prevVec.y += alpha*(vec.y - prevVec.y);
    prevVec.z += alpha*(vec.z - prevVec.z);
}

inline void filterVec(const floatVector3& vec, floatVector3& prevVec, float alpha, float thresh)
{
    float maxX = abs(vec.x);
    float maxY = abs(vec.y);
    float maxZ = abs(vec.z);
    float maxVal = maxX > maxY ? (maxX>maxZ?maxX:maxZ) : (maxY>maxZ?maxY:maxZ);
    if (maxVal <= thresh)
    {
        prevVec.x += alpha*(vec.x - prevVec.x);
        prevVec.y += alpha*(vec.y - prevVec.y);
        prevVec.z += alpha*(vec.z - prevVec.z);
    }
}


template <typename Tdata, typename Tcoeff=Tdata>
class SimpleIirFilter
{
public:
    SimpleIirFilter(Tcoeff alpha) : alpha_(alpha), prevVal_(Tdata())
    { }
    
    void init(const Tdata& data)
    {
        prevVal_ = data;
    }

    const Tdata& filterSample(const Tdata& x)
    {
        prevVal_ += alpha_*(x-prevVal_);
        return prevVal_;
    }

    const Tdata& getLastSample() const
    {
        return prevVal_;
    }

private:
    Tcoeff alpha_;
    Tdata prevVal_;
};


// M == len(a)
// N == len(b)
// a == coeffs on y_prev
// b == coeffs on x_prev
// pass in 'b' coeffs (the FIR coeffs) first. We're always assuming an initial 'a' coeff of 1.0, though
// sum(a[i]*y[t-i]) = sum(b[i]*x[t-i])
// or,
//   y[t] = x[t]*b0 + x[t-1]*b[0] + x[t-2]*b[1] + ... 
//                  - y[t-1]*a[0] - y[t-2]*b[1] ...

template <typename Tdata, int N, int M, typename Tcoeff=Tdata>
class IirFilter
{
public:
    IirFilter(const std::array<Tcoeff, N>& b, const std::array<Tcoeff, M>& a); // pass in a[1:], assuming a[0] == 1.0
    Tdata filterSample(const Tdata& x);

private:
    std::array<Tcoeff, M> a_; // feedback (y) coefficients
    std::array<Tcoeff, (N>0?N-1:0)> b_; // feed-forward (x) coefficients
    Tcoeff b0_ = Tcoeff(0); // first b coefficient (separate just to make code nicer in filterSample)

    RingBuffer<Tdata, M> y_prev_;
    RingBuffer<Tdata, (N>0?N-1:0)> x_prev_;

};

template <typename Tdata, int N, int M, typename Tcoeff>
IirFilter<Tdata,N,M,Tcoeff>::IirFilter(const std::array<Tcoeff, N>& b, const std::array<Tcoeff, M>& a)
{
    a_ = a;
    if(N > 0)
    {
        b0_ = b[0];
    }

    if(N > 1)
    {
        std::copy(b.begin()+1, b.end(), b_.begin());
    }
}

template <typename Tdata, int N, int M, typename Tcoeff>
Tdata IirFilter<Tdata,N,M,Tcoeff>::filterSample(const Tdata& x)
{
    Tdata y = b0_*x;
    for(int index = 0; index < (int)b_.size(); index++)
    {
        y += b_[index]*x_prev_[-index];
    }

    for(int index = 0; index < (int)a_.size(); index++)
    {
        y -= a_[index]*y_prev_[-index];
    }
    
    if(N > 1) x_prev_.push_back(x);
    if(M > 0) y_prev_.push_back(y);

    return y;
}
