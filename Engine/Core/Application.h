#pragma once

#include <Windows.h>

#include <memory>
#include <cstdint>

#include "EngineConfig.h"
#include "FrameLimiter.h"

class WinWindow;
class Engine;

class Application
{
public:
    Application();
    ~Application();

    Application(
        const Application&
    ) = delete;

    Application&
        operator=(
            const Application&
            ) = delete;

    bool Initialize(
        HINSTANCE hInstance,
        const EngineConfig& config
    );

    int Run();

    Engine& GetEngine();

    void SetTargetFPS(
        std::uint32_t fps
    );

    std::uint32_t GetTargetFPS() const;

    void SetVSync(
        bool enabled
    );

    void SetPaused(
        bool paused
    );

    bool IsPaused() const;

    const EngineConfig&
        GetConfig() const;

private:
    void ProcessPendingResize();

    void UpdateRuntimeStats(
        std::uint32_t fixedSteps
    );

    void UpdateWindowTitle();

private:
    std::unique_ptr<WinWindow>
        m_window;

    std::unique_ptr<Engine>
        m_engine;

    EngineConfig m_config;

    FrameLimiter m_frameLimiter;

    float m_fixedAccumulator = 0.0f;

    float m_titleUpdateTimer = 0.0f;

    bool m_comInitialized = false;

    bool m_isPaused = false;

    float m_prePauseTimeScale = 1.0f;
};