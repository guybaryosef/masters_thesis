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
struct lock_free_vector_iterator;

template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable<T>::value; 

// TODO: look over implementation
template <  typename T, 
            size_t FIRST_BUCKET_SIZE = 2, 
            size_t BUCKET_COUNT      = 16 >
class lock_free_vector
{
public:
    using Bucket = std::pair<size_t, std::atomic<T*>>;

    using value_type       = T;
    using size_type        = size_t;
    using reference        = value_type&;
    using const_reference  = const value_type&;
    using iterator         = lock_free_vector_iterator<value_type, Bucket>;
    using const_iterator   = lock_free_vector_iterator<const value_type, Bucket>;
    // using reverse_iterator = typename container_type::reverse_iterator;
    // using const_reverse_iterator = typename container_type::const_reverse_iterator;

    constexpr iterator begin()                         { return iterator(&(_bucketArr[0])); }
    constexpr const_iterator begin() const             { return const_iterator((Bucket *) &(_bucketArr[0])); }
    constexpr const_iterator cbegin() const            { return const_iterator((Bucket *) &(_bucketArr[0])); }
    constexpr iterator end()                           
    {
        auto [bucket, idx] = getLocation(size()); 
        return iterator(static_cast<Bucket *>(&(_bucketArr[bucket])), static_cast<std::ptrdiff_t>(idx)); 
    }
    constexpr const_iterator end() const               
    { 
        auto [bucket, idx] = getLocation(size()); 
        return const_iterator(const_cast<Bucket *>(&(_bucketArr[bucket])), idx); 
    }
    constexpr const_iterator cend() const              
    { 
        auto [bucket, idx] = getLocation(size()); 
        return const_iterator(const_cast<Bucket *>(&(_bucketArr[bucket])), idx); 
    }

private:
    struct WriteDescriptor
    {
        const value_type _val;
        const size_type  _position;
        bool             _completed;

        WriteDescriptor(const value_type& val_, const size_type pos_) : _val(val_), _position(pos_), _completed{false} {}
    };

    struct Descriptor
    {
        const size_type  _size;
        WriteDescriptor* _writeDescriptor;

        Descriptor() : _size{}, _writeDescriptor{nullptr} {}
        Descriptor(size_type size_, WriteDescriptor* writeDesc_) : _size(size_), _writeDescriptor(writeDesc_) {}

        ~Descriptor()
        {
            delete _writeDescriptor;
        }
    };

public:
    lock_free_vector() noexcept 
            : _desc (std::make_shared<Descriptor>())
            , _usedBucketCount {0} 
    {
        for (auto& i : _bucketArr)
        {
            i.first = 0;
            i.second.store(nullptr);
        }

        reserve(FIRST_BUCKET_SIZE);
    }

    ~lock_free_vector() noexcept
    {
        for(auto& i : _bucketArr)
            if (i.second)
                delete[] i.second;
    }

    T& operator[] (const size_type idx_)
    {
        return at(idx_);
    }

    void push_back(const value_type& val_)
    {
        std::shared_ptr<Descriptor> currDesc, nextDesc {};
        do
        {
            currDesc = std::atomic_load(&_desc);
            complete_write();

            auto bucket = highest_bit(currDesc->_size + FIRST_BUCKET_SIZE) - highest_bit(FIRST_BUCKET_SIZE);
            if (_bucketArr[bucket].second.load() == nullptr)
                allocate_bucket(bucket);
            
            auto writeDesc_ = new WriteDescriptor(val_, currDesc->_size);
            nextDesc = std::make_shared<Descriptor>(currDesc->_size+1, writeDesc_);
        }
        while (!std::atomic_compare_exchange_strong(&_desc, &currDesc, nextDesc));
        
        complete_write();
    }

    bool update(const size_type idx_, const value_type& val_)
    {
        if (likely(idx_ < size()))
        {
            at(idx_) = val_;
            return true;
        }
        return false;
    }

    value_type pop_back()
    {
        std::shared_ptr<Descriptor> currDesc, nextDesc {};
        value_type element {};
        do
        {
            currDesc = std::atomic_load(&_desc);
            complete_write();
            element = at(currDesc->_size-1);
            nextDesc = std::make_shared<Descriptor> (currDesc->_size-1, nullptr);
        }
        while (!std::atomic_compare_exchange_strong(&_desc, &currDesc, nextDesc));

        return element;
    }

    void reserve(const size_type size)
    {
        const auto newBucketSize = highest_bit(size + FIRST_BUCKET_SIZE - 1) - highest_bit(FIRST_BUCKET_SIZE);
        while (_usedBucketCount < static_cast<int>(newBucketSize))
            allocate_bucket(_usedBucketCount+1);
    }

    size_type size() const
    {
        auto currDesc = std::atomic_load_explicit(&_desc, std::memory_order_relaxed);
        int adjustment = (!currDesc->_writeDescriptor || currDesc->_writeDescriptor->_completed) ? 0 : 1;
        return currDesc->_size - adjustment;
    }

    size_t bucket_count() const
    {
        return _usedBucketCount + 1; // +1 because we are starting at 0
    }

private:
    std::pair<size_t, size_type> getLocation (const size_type i_) const
    {
        const size_type pos     = i_ + FIRST_BUCKET_SIZE;
        const auto      highBit = highest_bit(pos);
        const size_t    bucket  = highBit - highest_bit(FIRST_BUCKET_SIZE);
        const size_type idx     = pos ^ static_cast<uint64_t>(pow(2, highBit));
        return {bucket, idx};
    }

    value_type& at(const size_type i_)
    {
        auto [bucket, idx] = getLocation(i_); 
        T* arr = _bucketArr[bucket].second.load(); 
        return arr[idx];
    }

    void complete_write()
    {
        const auto currDesc  = std::atomic_load(&_desc);
        auto writeDesc = currDesc->_writeDescriptor;

        if (writeDesc && !writeDesc->_completed)
        {
            value_type& ele = at(writeDesc->_position); 
            ele = writeDesc->_val;
            writeDesc->_completed = true;
        }
    }

    size_type highest_bit(const size_type val_) const noexcept
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
        }
    }

    std::shared_ptr<Descriptor> _desc; // switch with std::atomic<shared_ptr> when P0718R2 gets into libstdc++
    std::array<Bucket, BUCKET_COUNT> _bucketArr;
    size_t _usedBucketCount;
};

// Iterator is modeled after a std::deque iterator. Very helpful
// stack overflow page: https://stackoverflow.com/questions/6292332/what-really-is-a-deque-in-stl 
template <typename T, typename Bucket = lock_free_vector<T>::Bucket>
struct lock_free_vector_iterator
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
    lock_free_vector_iterator (Bucket* bucket_, difference_type offset_ = 0) noexcept
    {
        set_bucket(bucket_);
        _cur = _first + offset_;
    }

    lock_free_vector_iterator& operator= (const lock_free_vector_iterator& iterator_)
    {
        _cur    = iterator_._cur;
        _first  = iterator_._first;
        _last   = iterator_._last;
        _bucket = iterator_._bucket;
    }

    lock_free_vector_iterator& operator++ () 
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

    lock_free_vector_iterator operator++ (int) 
    {
        lock_free_vector_iterator returnIt = *this; 
        ++(*this); 
        return returnIt;
    }

    lock_free_vector_iterator& operator-- ()
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

    lock_free_vector_iterator operator-- (int)
    {
        lock_free_vector_iterator returnIt = *this;
        --(*this);
        return returnIt;
    }

    bool operator== (const lock_free_vector_iterator& other) const noexcept
    {
        return _cur == other._cur;
    }

    bool operator!= (const lock_free_vector_iterator& other) const noexcept
    {
        return !(*this == other);
    }

    reference operator* () const noexcept
    {
        return *_cur;
    }

    lock_free_vector_iterator& operator+= (difference_type n)
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
    
    lock_free_vector_iterator& operator-= (difference_type n)
    {
        return *this += -n;
    }

    lock_free_vector_iterator operator+ (difference_type n) const
    {
        lock_free_vector_iterator copy = *this;
        return copy += n;
    }

    lock_free_vector_iterator operator- (difference_type n) const
    {
        lock_free_vector_iterator copy = *this;
        return copy -= n;
    }

    lock_free_vector_iterator operator[] (difference_type n) const
    {
        return *(*this + n);
    }
};


} // namespace gby