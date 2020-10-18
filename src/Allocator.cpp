#include "Allocator.h"

void *operator new(size_t size, EResourceType type)
{
    void *p = malloc(size);
    MemTracker::track(p, type, size);
    return p;
}

// void operator delete(void *p)
// {
//     MemTracker::untrack(p);
//     free(p);
// }
