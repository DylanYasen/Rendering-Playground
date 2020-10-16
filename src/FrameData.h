#pragma once
#include <cstdint>

struct FrameData
{
    typedef std::uint64_t uint64;

public:
    uint64 last = 0;
    uint64 now = 0;
    uint64 freq = 0;
    double deltaTime = 0;
    float frametimes[60] = {0};

    FrameData(uint64 perfPref);
    void Stamp(uint64 perfCounter);
};
