#include "Allocator.h"

std::unordered_map<EResourceType, size_t> MemTracker::resourceMap;
std::unordered_map<uintptr_t, MemTracker::Chunk> MemTracker::addressMap;
std::unordered_map<uint16_t, MemTracker::Chunk>  MemTracker::resourceIDMap;

void *operator new(size_t size, EResourceType type)
{
    void *p = malloc(size);
    MemTracker::track(p, type, size);
    return p;
}

void operator delete(void *p)
{
    MemTracker::untrack(p);
    free(p);
}