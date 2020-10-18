#pragma once

#include <unordered_map>
#include <stdint.h>

#include "MemTracker.h"

template <typename T>
class Allocator
{
public:
    typedef T value_type;

    Allocator() = default;
    template <class U>
    constexpr Allocator(const Allocator<U> &) noexcept {}

    static T *allocate(std::size_t n)
    {

        T *p = std::allocator<T>{}.allocate(n);

        // todo: categorize container allocation.
        // need a cleaner way to get the type in,  interface already looks crazy right now
        MemTracker::track(p, EResourceType::Geometry, n * sizeof(T));

        return p;
    }

    static void deallocate(T *p, std::size_t n)
    {
        MemTracker::untrack(p);
        std::allocator<T>{}.deallocate(p, n);
    }
};

//void *operator new(size_t size);
void *operator new(size_t size, EResourceType type);
// void operator delete(void *p);
