#pragma once

#include <cstdint>

struct DebugStats
{
    float fps = 0.0f;

    float frameTimeMs = 0.0f;

    int entityCount = 0;

    int drawCalls = 0;

    std::uint32_t fixedSteps = 0;

    float fixedUpdateHz = 60.0f;

    float interpolationAlpha = 0.0f;

    bool vsync = false;

    // 0 == Unlimited
    std::uint32_t targetFPS = 0;

    int renderCommands = 0;
};