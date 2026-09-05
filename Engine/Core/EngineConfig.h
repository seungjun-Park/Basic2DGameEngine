#pragma once

#include <cstdint>
#include <string>

enum class WindowMode
{
    Windowed,
    BorderlessFullscreen
};

struct EngineConfig
{
    std::wstring windowTitle =
        L"Basic2DGameEngine";

    int windowWidth = 1280;
    int windowHeight = 720;

    WindowMode windowMode =
        WindowMode::Windowed;

    bool vsync = false;

    std::uint32_t targetFPS = 144;

    float fixedUpdateHz = 60.0f;

    std::uint32_t maxFixedSteps = 5;
    float maxDeltaTime = 0.1f;

    bool pauseWhenUnfocused = true;

    bool showDebugCollider = true;
    bool showRuntimeStats = true;
};