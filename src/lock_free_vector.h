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

template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable<T>::value; 

// TODO: look over implementation
template <  typename T, 
            size_t FIRST_BUCKET_SIZE = 2, 
            size_t BUCKET_COUNT      = 16 >
class lock_free_vector
{
    struct WriteDescriptor
    {
        const T      _val;
        const size_t _position;
        bool         _completed;

        WriteDescriptor(const T& val_, const size_t pos_) : _val(val_), _position(pos_), _completed{false} {}
    };

    struct Descriptor
    {
        const size_t     _size;
        WriteDescriptor* _writeDescriptor;

        Descriptor() : _size{}, _writeDescriptor{nullptr} {}
        Descriptor(size_t size_, WriteDescriptor* writeDesc_) : _size(size_), _writeDescriptor(writeDesc_) {}

        ~Descriptor()
        {
            delete _writeDescriptor;
        }
    };

public:
    lock_free_vector() : _desc (std::make_shared<Descriptor>())
    {
        for (auto& i : _memoryArr)
            i.store(nullptr);
    }

    ~lock_free_vector()
    {
        for(auto& i : _memoryArr)
            if (i)
                delete[] i;
    }

    T& operator[] (const size_t idx_)
    {
        return at(idx_);
    }

    void push_back(const T& val_)
    {
        std::shared_ptr<Descriptor> currDesc, nextDesc {};
        do
        {
            currDesc = std::atomic_load(&_desc);
            complete_write();

            auto bucket = highest_bit(currDesc->_size + FIRST_BUCKET_SIZE) - highest_bit(FIRST_BUCKET_SIZE);
            if (_memoryArr[bucket].load() == nullptr)
                allocate_bucket(bucket);
            
            auto writeDesc_ = new WriteDescriptor(val_, currDesc->_size);
            nextDesc = std::make_shared<Descriptor>(currDesc->_size+1, writeDesc_);
        }
        while (!std::atomic_compare_exchange_strong(&_desc, &currDesc, nextDesc));
        
        complete_write();
    }

    bool update(const size_t idx_, const T& val_)
    {
        if (likely(idx_ < size()))
        {
            at(idx_) = val_;
            return true;
        }
        return false;
    }

    T pop_back()
    {
        std::shared_ptr<Descriptor> currDesc, nextDesc {};
        T element {};
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

    void reserve(const size_t size)
    {
        auto i = highest_bit(std::atomic_load(&_desc)->_size + FIRST_BUCKET_SIZE - 1) - highest_bit(FIRST_BUCKET_SIZE);
        if (i < 0)
            i = 0;

        while (i < (highest_bit(size + FIRST_BUCKET_SIZE - 1) - highest_bit(FIRST_BUCKET_SIZE)) )
            allocate_bucket(++i);
    }

    size_t size() const
    {
        auto currDesc = std::atomic_load_explicit(&_desc, std::memory_order_relaxed);
        int adjustment =   (!currDesc                    || 
                            !currDesc->_writeDescriptor  || 
                            currDesc->_writeDescriptor->_completed == true) ? 0 : 1;
        return currDesc->_size - adjustment;
    }

private:
    T& at(const size_t i_)
    {
        uint64_t pos  = i_ + FIRST_BUCKET_SIZE;
        auto highBit  = highest_bit(pos);
        size_t bucket = highBit - highest_bit(FIRST_BUCKET_SIZE);
        T* arr = _memoryArr[bucket].load(); 
        size_t idx = pos ^ static_cast<uint64_t>(pow(2, highBit));
        return arr[idx];
    }

    void complete_write()
    {
        auto currDesc  = std::atomic_load(&_desc);
        auto writeDesc = currDesc->_writeDescriptor;

        if (writeDesc && !writeDesc->_completed)
        {
            T& ele = at(writeDesc->_position); 
            ele = writeDesc->_val;
            writeDesc->_completed = true;
        }
    }

    size_t highest_bit(const size_t val_) const
    {
        assert(val_ != 0);
        return 31-__builtin_clz(val_);
    }

    void allocate_bucket(const size_t bucket_)
    {
        T* L_VALUE_NULLPTR = nullptr;

        if (bucket_ >= BUCKET_COUNT)
            throw std::length_error("Lock-free array reached max bucket size."); 
        
        size_t bucketSize = pow(FIRST_BUCKET_SIZE, bucket_+1);
        T* newMemBlock = new T[bucketSize]();
        if (!_memoryArr[bucket_].compare_exchange_strong(L_VALUE_NULLPTR, newMemBlock))
            delete []newMemBlock;
    }

    std::shared_ptr<Descriptor> _desc; // switch with std::atomic<shared_ptr> when P0718R2 gets into libstdc++
    std::array<std::atomic<T*>, BUCKET_COUNT> _memoryArr;
};

} // namespace gby