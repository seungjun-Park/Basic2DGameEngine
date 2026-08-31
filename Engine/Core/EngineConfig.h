#pragma once

#include <cstdint>

enum class WindowMode
{
    Windowed,
    BorderlessFullscreen
};

struct EngineConfig
{
    // Window
    int windowWidth = 1280;
    int windowHeight = 720;

    WindowMode windowMode =
        WindowMode::Windowed;

    // Rendering
    bool vsync = false;

    // 0 = Unlimited
    std::uint32_t targetFPS = 144;

    // Fixed Update
    float fixedUpdateHz = 60.0f;

    // 한 Render Frame에서 실행할
    // 최대 FixedUpdate 횟수
    std::uint32_t maxFixedSteps = 5;

    // 지나치게 큰 frame delta 방지
    float maxDeltaTime = 0.1f;

    // Window Focus
    bool pauseWhenUnfocused = true;

    // Debug
    bool showDebugCollider = true;
    bool showRuntimeStats = true;
};