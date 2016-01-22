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
    arr_.fill(T());
}

template <typename T, int N>
T ring_buffer<T,N>::operator[](int index) const
{
    // allow negative indices
    index = (index + curr_pos_) % (int)arr_.size(); // yikes! need to cast size to signed value, otherwise % behaves differently
    if (index < 0) index += arr_.size();
    return arr_[index];
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

// specialization for zero-length buffer (which hopefully never happens)
template <typename T>
class ring_buffer<T,0>
{
public:
    ring_buffer();
    T operator[](int index) const;
    void push_back(const T& val);
    size_t size() const;
};
    
template <typename T>
ring_buffer<T,0>::ring_buffer()
{
}

template <typename T>
T ring_buffer<T,0>::operator[](int) const
{
//    static_assert(false, ":(");
    return T();
}

template <typename T>
void ring_buffer<T,0>::push_back(const T&)
{
//    static_assert(false, ":(");
}

template <typename T>
size_t ring_buffer<T,0>::size() const
{
    return 0;
}
