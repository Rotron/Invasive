#pragma once
#include <QVector>

template <class T>
class RingBuffer
{
public:
    class const_iterator {
    public:
        const_iterator(const RingBuffer& ref, int offset);

        const T& operator*();
        bool operator!=(const const_iterator&) const;
        const_iterator& operator++();

        friend const_iterator operator+(const const_iterator it, int rhs)
        {
            return const_iterator(it.ref_, it.offset_ + rhs);
        }

        friend const_iterator operator-(const const_iterator it, int rhs)
        {
            return const_iterator(it.ref_, it.offset_ - rhs);
        }

    private:
        const RingBuffer& ref_;
        int offset_;

    };

public:
    RingBuffer(size_t capacity = 0);
    void push_back(T value);
    void set_capacity(size_t capacity);
    T operator[](size_t offset) const;

    size_t capacity() const;
    size_t size() const;

    T sum() const;
    T average() const;

    const_iterator begin() const;
    const_iterator end() const;

private:
    QVector<T> buffer_;
    size_t size_;
    size_t begin_;
    size_t end_;
    T sum_;

};

template <class T>
RingBuffer<T>::const_iterator::const_iterator(const RingBuffer& ref, int offset) :
    ref_(ref),
    offset_(offset)
{

}

template <class T>
const T& RingBuffer<T>::const_iterator::operator*()
{
    return ref_.buffer_[(ref_.begin_ + offset_) % ref_.buffer_.size()];
}

template <class T>
bool RingBuffer<T>::const_iterator::operator!=(const const_iterator& rhs) const
{
    return rhs.offset_ != offset_;
}

template <class T>
typename RingBuffer<T>::const_iterator& RingBuffer<T>::const_iterator::operator++()
{
    offset_++;
    return *this;
}

template <class T>
RingBuffer<T>::RingBuffer(size_t capacity) :
    buffer_(capacity),
    size_(0),
    begin_(0),
    end_(0),
    sum_()
{

}

template <class T>
void RingBuffer<T>::push_back(T value)
{
    T end_sum = buffer_[end_];
    buffer_[end_] = value;
    sum_ += value;
    end_ = (end_ + 1) % buffer_.size();
    if (size_ < static_cast<size_t>(buffer_.size())) {
        size_++;
    }
    else {
        begin_ = (begin_ + 1) % buffer_.size();
        sum_ -= end_sum;
    }
}

template <class T>
void RingBuffer<T>::set_capacity(size_t capacity)
{
    buffer_.resize(capacity);
}

template <class T>
T RingBuffer<T>::operator[](size_t offset) const
{
    return *(begin() + offset);
}

template <class T>
size_t RingBuffer<T>::capacity() const
{
    return buffer_.size();
}

template <class T>
size_t RingBuffer<T>::size() const
{
    return size_;
}


template <class T>
T RingBuffer<T>::sum() const
{
    return sum_;
}

template <class T>
T RingBuffer<T>::average() const
{
    if (size() > 0) {
        return sum_ / size();
    }
    else {
        return T();
    }
}

template <class T>
typename RingBuffer<T>::const_iterator RingBuffer<T>::begin() const
{
    return const_iterator(*this, 0);
}

template <class T>
typename RingBuffer<T>::const_iterator RingBuffer<T>::end() const
{
    return const_iterator(*this, size());
}
