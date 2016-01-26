#pragma once

#include <array>

template <typename T, int N>
class ringBuffer
{
public:
    ringBuffer();
    T operator[](int index) const;
    void push_back(const T& val);
    size_t size() const;

private:    
    std::array<T, N> arr_;
    int curr_pos_ = 0;
};


template <typename T, int N>
ringBuffer<T,N>::ringBuffer()
{
    arr_.fill(T());
}

template <typename T, int N>
T ringBuffer<T,N>::operator[](int index) const
{
    // allow negative indices
    index = (index + curr_pos_) % (int)arr_.size(); // yikes! need to cast size to signed value, otherwise % behaves differently
    if (index < 0) index += arr_.size();
    return arr_[index];
}

template <typename T, int N>
void ringBuffer<T,N>::push_back(const T& val)
{
    curr_pos_ = (curr_pos_+1)%arr_.size();
    arr_[curr_pos_] = val;
}

template <typename T, int N>
size_t ringBuffer<T,N>::size() const
{
    return arr_.size();
}

// specialization for zero-length buffer (which hopefully never happens)
template <typename T>
class ringBuffer<T,0>
{
public:
    ringBuffer();
    T operator[](int index) const;
    void push_back(const T& val);
    size_t size() const;
};
    
template <typename T>
ringBuffer<T,0>::ringBuffer()
{
}

template <typename T>
T ringBuffer<T,0>::operator[](int) const
{
//    static_assert(false, ":(");
    return T();
}

template <typename T>
void ringBuffer<T,0>::push_back(const T&)
{
//    static_assert(false, ":(");
}

template <typename T>
size_t ringBuffer<T,0>::size() const
{
    return 0;
}
