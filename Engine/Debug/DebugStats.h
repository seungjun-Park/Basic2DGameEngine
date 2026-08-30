#pragma once

struct DebugStats
{
    int entityCount = 0;
    int drawCalls = 0;

    void ResetPerFrame()
    {
        drawCalls = 0;
    }
};