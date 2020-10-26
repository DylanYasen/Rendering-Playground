#include "FrameData.h"
#include <iostream>

uint32_t Time::totalTimeMs;
float Time::totalTimeSec;
float Time::deltaTime;

FrameData::FrameData(uint64 perfPref)
    : freq(perfPref)
{
}

void FrameData::Stamp(uint64 perfCounter)
{
    last = now;
    now = perfCounter;
    deltaTime = (double)((now - last) * 1000 / (double)freq);

    const int sampleCount = 60;
    for (int i = 0; i < sampleCount - 1; i++)
        frametimes[i] = frametimes[i + 1];
    frametimes[sampleCount - 1] = deltaTime;
}