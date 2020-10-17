#pragma once

#include <unordered_map>
#include <stdint.h>

enum class EResourceType
{
    Generic = 0,
    Texture = 1,
    Geometry = 2,
    Max = 3
};

static const char *ResourceTypeToStr(EResourceType e)
{
    switch (e)
    {
    case EResourceType::Generic:
        return "Generic";

    case EResourceType::Texture:
        return "Texture";

    case EResourceType::Geometry:
        return "Geometry";

    default:
        return "N/A";
    }
}

class MemTracker
{

public:
    struct Chunk
    {
        Chunk() = default;
        Chunk(size_t s, EResourceType t) : size(s), type(t){};

        size_t size;
        EResourceType type;
    };

    static void track(const void *p, EResourceType type, size_t size)
    {
        if (!p)
            return;

        resourceMap[type] += size;

        Chunk c(size, type);
        addressMap[(uintptr_t)p] = c;
    }

    static void track(uint16_t resourceID, EResourceType type, size_t size)
    {
        resourceMap[type] += size;

        Chunk c(size, type);
        resourceIDMap[resourceID] = c;
    }

    static void untrack(void *p)
    {
        if (!p)
            return;

        if (addressMap.find((uintptr_t)p) != addressMap.end())
        {
            const Chunk &c = addressMap[(uintptr_t)p];
            resourceMap[c.type] -= c.size;
        }
    }

    static void untrack(uint16_t resourceID)
    {
        if (resourceIDMap.find(resourceID) != resourceIDMap.end())
        {
            const Chunk &c = addressMap[resourceID];
            resourceMap[c.type] -= c.size;
        }
    }

    static std::unordered_map<EResourceType, size_t> resourceMap;

    // todo: resource id could collide.
    static std::unordered_map<uint16_t, Chunk> resourceIDMap;
    static std::unordered_map<uintptr_t, Chunk> addressMap;
};

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
        MemTracker::track(p, EResourceType::Geometry, n);

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
void operator delete(void *p);
