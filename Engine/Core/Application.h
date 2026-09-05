#pragma once

#include <Windows.h>

#include <memory>
#include <cstdint>
#include <string>

#include "EngineConfig.h"
#include "FrameLimiter.h"

class WinWindow;
class Engine;
struct ProjectConfig;

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

    int Run(
        const ProjectConfig& projectConfig,
        const std::wstring& projectConfigPath
    );

    void SetTargetFPS(
        std::uint32_t fps
    );
    void SetVSync(
        bool enabled
    );
    void SetPaused(
        bool paused
    );


    Engine& GetEngine();
    std::uint32_t GetTargetFPS() const;
    const EngineConfig&
        GetConfig() const;

    bool IsPaused() const;

private:
    void ProcessPendingResize();

    void UpdateRuntimeStats(
        std::uint32_t fixedSteps
    );

    void ReportProfilerSpikeIfNeeded();

    void UpdateWindowTitle();

    void ApplyLiveProjectSettings(
        const ProjectConfig& config
    );

private:
    std::unique_ptr<WinWindow>
        m_window;

    std::unique_ptr<Engine>
        m_engine;

    EngineConfig m_config;

    FrameLimiter m_frameLimiter;

    float m_fixedAccumulator = 0.0f;

    float m_titleUpdateTimer = 0.0f;

    bool m_showProfilerTitle = false;

    bool m_comInitialized = false;

    bool m_isPaused = false;

    float m_prePauseTimeScale = 1.0f;
};