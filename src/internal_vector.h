/*
 * lock_free_vector - This is a lock free vector
 * implementation based on The paper called
 * "Lock-free Dynamically Resizable Arrays" by 
 * Damian Dechev, Peter Pirkelbauer, and Bjarne 
 * Stroustrup.
 * 
 * https://www.stroustrup.com/lock-free-vector.pdf
 *
 */

#pragma once

#include <vector>
#include <math.h>
#include <concepts>
#include <atomic>

#include "utils.h"

namespace gby
{

template <typename T, typename Bucket>
struct internal_vector_iterator;

// This highly resembles a deque. Its use is to 
// be *like* a vector, except that once it reached
// capacity it doesn't move any existing data but
// rather allocates a new memory block. 
// What sets this vector apart from std::deque is a
// couple of things:
// 1. It only allocates to the end (so it is single-sided).
// 2. The allocation is done in a lock-free manner.
// 3. The bucket sizes grow exponentially.
template <  typename T, 
            size_t FIRST_BUCKET_SIZE = 2, 
            size_t BUCKET_COUNT      = 16 >
class internal_vector
{
public:
    using Bucket = std::pair<size_t, std::atomic<T*>>; // <bucket size, pointer to data>

    using value_type       = T;
    using size_type        = size_t;
    using reference        = value_type&;
    using const_reference  = const value_type&;
    using iterator         = internal_vector_iterator<value_type, Bucket>;
    using const_iterator   = internal_vector_iterator<const value_type, Bucket>;
    
    constexpr iterator begin()              { return iterator(&(_bucketArr[0]));                  }
    constexpr const_iterator begin()  const { return const_iterator((Bucket *) &(_bucketArr[0])); }
    constexpr const_iterator cbegin() const { return const_iterator((Bucket *) &(_bucketArr[0])); }
    constexpr iterator end()                           
    {
        auto [bucket, idx] = get_location(size()); 
        return iterator(static_cast<Bucket *>(&(_bucketArr[bucket])), static_cast<std::ptrdiff_t>(idx)); 
    }
    constexpr const_iterator end() const               
    { 
        auto [bucket, idx] = get_location(size()); 
        return const_iterator(const_cast<Bucket *>(&(_bucketArr[bucket])), idx); 
    }
    constexpr const_iterator cend() const              
    { 
        auto [bucket, idx] = get_location(size()); 
        return const_iterator(const_cast<Bucket *>(&(_bucketArr[bucket])), idx); 
    }

public:
    internal_vector() noexcept 
            : _size {}
            , _capacity {}
            , _usedBucketCount {} 
    {
        for (auto& i : _bucketArr)
        {
            i.first = 0;
            i.second.store(nullptr);
        }

        reserve(FIRST_BUCKET_SIZE);
    }

    ~internal_vector() noexcept
    {
        for(auto& i : _bucketArr)
            if (i.second)
                delete[] i.second;
    }

    constexpr T& operator[] (const size_type idx_)
    {
        return at(idx_);
    }

    constexpr const T& operator[] (const size_type idx_) const
    {
        return at(idx_);
    }

    constexpr const size_type push_back(const value_type& val_)
    {
        const size_type index = _size.fetch_add(1);
        const auto [bucket, b_idx] = get_location(index);
        if (_bucketArr[bucket].first == 0)
            allocate_bucket(bucket);
        
        _bucketArr[bucket].second.load()[b_idx] = val_; 

        return index;        
    }

    constexpr bool update(const size_type idx_, const value_type& val_)
    {
        if (likely(idx_ < size()))
        {
            at(idx_) = val_;
            return true;
        }
        return false;
    }

    constexpr value_type pop_back()
    {
        value_type element  {};
        size_type  cur_size {};
        do
        {
            cur_size = _size.load(std::memory_order_relaxed);
            if (unlikely(cur_size == 0))
            {
                throw std::out_of_range("internal vector is empty!");
            }

            element  = at(cur_size-1);
        }
        while (!_size.compare_exchange_strong(cur_size, cur_size-1));
        
        return element;
    }

    constexpr void reserve(const size_type size)
    {
        const size_t newBucketSize = highest_bit(size + FIRST_BUCKET_SIZE - 1) - highest_bit(FIRST_BUCKET_SIZE);
        while (_usedBucketCount < newBucketSize)
            allocate_bucket(_usedBucketCount+1);
    }

    constexpr size_type size() const
    {
        return _size.load(std::memory_order_relaxed);
    }

    constexpr size_type capacity() const
    {
        return _capacity;
    }

    constexpr size_t bucket_count() const
    {
        return _usedBucketCount + 1; // +1 because we are starting at 0
    }

private:
    constexpr std::pair<size_t, size_type> get_location (const size_type i_) const
    {
        const size_type pos     = i_ + FIRST_BUCKET_SIZE;
        const size_t    highBit = highest_bit(pos);
        const size_t    bucket  = highBit - highest_bit(FIRST_BUCKET_SIZE);
        const size_type idx     = pos ^ static_cast<uint64_t>(pow(2, highBit));
        return {bucket, idx};
    }

    constexpr value_type& at(const size_type i_)
    {
        if (unlikely(i_ >= size()))
        {
            throw std::out_of_range("index " + std::to_string(i_) + " outside of size " + std::to_string(size()) + ".");
        }
        auto [bucket, idx] = get_location(i_); 
        T*   arr           = _bucketArr[bucket].second.load(); 
        return arr[idx];
    }

    constexpr size_t highest_bit(const size_type val_) const noexcept
    {
        assert(val_ != 0);
        return 31-__builtin_clz(val_);
    }

    void allocate_bucket(const size_type bucket_)
    {
        value_type* L_VALUE_NULLPTR = nullptr;

        if (bucket_ >= BUCKET_COUNT)
            throw std::length_error("Lock-free array reached max bucket size."); 
        
        const size_t bucketSize = pow(FIRST_BUCKET_SIZE, bucket_+1);
        T* newMemBlock = new T[bucketSize]();
        if (!_bucketArr[bucket_].second.compare_exchange_strong(L_VALUE_NULLPTR, newMemBlock))
        {
            delete []newMemBlock;
        }
        else
        {
            _bucketArr[bucket_].first = bucketSize;
            _usedBucketCount = bucket_;
            _capacity += bucketSize;
        }
    }

    std::array<Bucket, BUCKET_COUNT> _bucketArr;

    std::atomic<size_type> _size;
    size_type              _capacity;
    size_t                 _usedBucketCount;
};

// Iterator is modeled after a std::deque iterator. Very helpful
// stack overflow page: https://stackoverflow.com/questions/6292332/what-really-is-a-deque-in-stl 
template <typename T, typename Bucket = internal_vector<T>::Bucket>
struct internal_vector_iterator
{
    // iterator traits
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::random_access_iterator_tag;
    
    pointer _cur;
    pointer _first;
    pointer _last;

private:
    Bucket* _bucket;

    void set_bucket(Bucket* bucket_)
    {
        _bucket = bucket_;
        _first = &(bucket_->second[0]);
        _last = _first + (bucket_->first - 1); 
    }

public:
    internal_vector_iterator (Bucket* bucket_, difference_type offset_ = 0) noexcept
    {
        set_bucket(bucket_);
        _cur = _first + offset_;
    }

    internal_vector_iterator& operator= (const internal_vector_iterator& iterator_)
    {
        _cur    = iterator_._cur;
        _first  = iterator_._first;
        _last   = iterator_._last;
        _bucket = iterator_._bucket;
    }

    internal_vector_iterator& operator++ () 
    {
        if (_cur == _last)
        {     
            set_bucket(_bucket + 1);
            _cur = _first;
        }
        else
        {
            ++_cur;
        }
        return *this;
    }

    internal_vector_iterator operator++ (int) 
    {
        internal_vector_iterator returnIt = *this; 
        ++(*this); 
        return returnIt;
    }

    internal_vector_iterator& operator-- ()
    {
        if(_cur == _first)
        {
            set_bucket(_bucket - 1);
            _cur = _last;
        }
        else
        {
            --_cur;
        }
        return *this;
    }

    internal_vector_iterator operator-- (int)
    {
        internal_vector_iterator returnIt = *this;
        --(*this);
        return returnIt;
    }

    bool operator== (const internal_vector_iterator& other) const noexcept
    {
        return _cur == other._cur;
    }

    bool operator!= (const internal_vector_iterator& other) const noexcept
    {
        return !(*this == other);
    }

    reference operator* () const noexcept
    {
        return *_cur;
    }

    internal_vector_iterator& operator+= (difference_type n)
    {
        difference_type offset = n + (_cur - _first);
        if (offset >= 0 && offset < _bucket->first) // if in same bucket
        {
            _cur += n;
        }
        else
        {
            Bucket* bucket = _bucket + 1;
            difference_type offsettedByBuckets {_last - _cur + 1}; // +1 for the bump to enter the next bucket
        
            while (offset - offsettedByBuckets > bucket->first) 
            {
                offsettedByBuckets += bucket->first;
                ++bucket;
            }
            
            set_bucket(bucket);
            _cur = _first + (offset - offsettedByBuckets);
        }

        return *this;
    }
    
    internal_vector_iterator& operator-= (difference_type n)
    {
        return *this += -n;
    }

    internal_vector_iterator operator+ (difference_type n) const
    {
        internal_vector_iterator copy = *this;
        return copy += n;
    }

    internal_vector_iterator operator- (difference_type n) const
    {
        internal_vector_iterator copy = *this;
        return copy -= n;
    }

    internal_vector_iterator operator[] (difference_type n) const
    {
        return *(*this + n);
    }
};


} // namespace gby