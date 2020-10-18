#include "MemTracker.h"

std::unordered_map<uint8_t, size_t> MemTracker::resourceMap;
std::unordered_map<uintptr_t, MemTracker::Chunk> MemTracker::addressMap;
std::unordered_map<uint16_t, MemTracker::Chunk> MemTracker::resourceIDMap;

void MemTracker::track(const void *p, EResourceType type, size_t size)
{
    if (!p)
        return;

    resourceMap[(uint8_t)type] += size;

    uintptr_t addr = reinterpret_cast<uintptr_t>(p);

    addressMap[addr] = Chunk(size, type);
}

void MemTracker::track(uint16_t resourceID, EResourceType type, size_t size)
{
    resourceMap[(uint8_t)type] += size;
    resourceIDMap[resourceID] = Chunk(size, type);
}

void MemTracker::untrack(void *p)
{
    if (!p)
        return;

    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    auto itr = addressMap.find(addr);
    if (itr != addressMap.end())
    {
        const Chunk &c = itr->second;
        resourceMap[(uint8_t)c.type] -= c.size;

        addressMap.erase(itr);
    }
}

void MemTracker::untrack(uint16_t resourceID)
{
    auto itr = resourceIDMap.find(resourceID);
    if (itr != resourceIDMap.end())
    {
        const Chunk &c = addressMap[resourceID];
        resourceMap[(uint8_t)c.type] -= c.size;

        resourceIDMap.erase(itr);
    }
}