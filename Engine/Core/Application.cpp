#include "Application.h"

#include "Engine.h"
#include "Time.h"

#include "Engine/Platform/Windows/WinWindow.h"
#include "Engine/Platform/Windows/WinInput.h"
#include "Engine/Debug/DebugStats.h"

#include <algorithm>
#include <cstdint>

Application::Application() = default;

Application::~Application()
{
    // Engine / Window가 COM보다
    // 먼저 파괴되도록 명시적으로 정리
    m_engine.reset();
    m_window.reset();

    if (m_comInitialized)
    {
        CoUninitialize();

        m_comInitialized = false;
    }
}

bool Application::Initialize(
    HINSTANCE hInstance,
    const EngineConfig& config)
{
    m_config =
        config;

    HRESULT hr =
        CoInitializeEx(
            nullptr,
            COINIT_MULTITHREADED
        );

    if (FAILED(hr))
    {
        return false;
    }

    m_comInitialized = true;

    m_window =
        std::make_unique<WinWindow>();

    if (!m_window->Initialize(
        hInstance,
        m_config.windowWidth,
        m_config.windowHeight,
        L"Dobi2D"))
    {
        return false;
    }

    Time::Initialize(
        m_config.fixedUpdateHz,
        m_config.maxDeltaTime
    );

    WinInput::Initialize();

    m_engine =
        std::make_unique<Engine>();

    if (!m_engine->Initialize(
        *m_window))
    {
        return false;
    }

    return true;
}

int Application::Run()
{
    while (true)
    {
        //
        // Frame budget에는 message pump까지
        // 전체 frame 작업이 포함되어야 한다.
        //
        m_frameLimiter.BeginFrame();

        if (!m_window->ProcessMessages())
        {
            break;
        }

        Time::Tick();

        ProcessPendingResize();

        // 최소화 또는 Focus가 없는 상태
        if (
            m_window->IsMinimized() ||
            (
                m_config.pauseWhenUnfocused &&
                !m_window->IsActive()
                )
            )
        {
            WinInput::Reset();

            m_fixedAccumulator =
                0.0f;

            Sleep(10);

            Time::ResetFrameTimer();

            continue;
        }

        WinInput::Update();

        m_engine->
            BeginProfileFrame();

        const float fixedDelta =
            Time::FixedDeltaTime();

        const float maxAccumulator =
            fixedDelta *
            static_cast<float>(
                m_config.maxFixedSteps
                );

        m_fixedAccumulator +=
            Time::DeltaTime();

        // Spiral of Death 방지
        m_fixedAccumulator =
            min(
                m_fixedAccumulator,
                maxAccumulator
            );

        std::uint32_t fixedSteps = 0;

        while (
            m_fixedAccumulator >=
            fixedDelta
            )
        {
            m_engine->FixedUpdate(
                fixedDelta
            );

            m_fixedAccumulator -=
                fixedDelta;

            ++fixedSteps;
        }

        float interpolationAlpha =
            0.0f;

        if (fixedDelta > 0.0f)
        {
            interpolationAlpha =
                m_fixedAccumulator /
                fixedDelta;
        }

        m_engine->
            SetInterpolationAlpha(
                interpolationAlpha
            );

        m_engine->Update(
            Time::DeltaTime()
        );

        m_engine->LateUpdate(
            Time::DeltaTime()
        );

        m_engine->Render(
            m_config.vsync
        );

        m_engine->
            EndProfileFrame();

        UpdateRuntimeStats(
            fixedSteps
        );

        UpdateWindowTitle();

        // VSync가 Present에서 기다리므로
        // CPU FrameLimiter를 동시에 사용하지 않음
        if (!m_config.vsync)
        {
            m_frameLimiter.EndFrame(
                m_config.targetFPS
            );
        }
    }

    return 0;
}

void Application::UpdateRuntimeStats(
    std::uint32_t fixedSteps)
{
    DebugStats& stats =
        m_engine->GetDebugStats();

    stats.fps =
        Time::FPS();

    stats.frameTimeMs =
        Time::UnscaledDeltaTime() *
        1000.0f;

    stats.fixedSteps =
        fixedSteps;

    stats.fixedUpdateHz =
        1.0f /
        Time::FixedDeltaTime();

    stats.interpolationAlpha =
        m_engine->
        GetInterpolationAlpha();

    stats.vsync =
        m_config.vsync;

    stats.targetFPS =
        m_config.targetFPS;
}

void Application::UpdateWindowTitle()
{
    if (!m_config.showRuntimeStats)
    {
        return;
    }

    m_titleUpdateTimer +=
        Time::UnscaledDeltaTime();

    // WinAPI title을 매 frame 갱신하지 않는다.
    if (m_titleUpdateTimer <
        0.25f)
    {
        return;
    }

    m_titleUpdateTimer = 0.0f;

    const DebugStats& stats =
        m_engine->GetDebugStats();

    wchar_t targetText[32]{};

    if (stats.targetFPS == 0)
    {
        wcscpy_s(
            targetText,
            L"Unlimited"
        );
    }
    else
    {
        swprintf_s(
            targetText,
            L"%u",
            stats.targetFPS
        );
    }

    float averageBatchSize =
        0.0f;

    if (stats.renderBatches > 0)
    {
        averageBatchSize =
            static_cast<float>(
                stats.batchedRenderCommands
                ) /
            static_cast<float>(
                stats.renderBatches
                );
    }

    wchar_t title[768]{};

    swprintf_s(
        title,

        L"Demo | "
        L"FPS %.1f | "
        L"Frame %.2f ms | "
        L"CPU %.2f ms | "
        L"F %.2f/%.2f ms (%u) | "
        L"U %.2f | "
        L"L %.2f | "
        L"R %.2f | "
        L"P %.2f | "
        L"Fixed %.0f Hz (%u) | "
        L"Ent %d | "
        L"Cmd %d | "
        L"Batch %u Avg %.1f Max %u | "
        L"Draw %d | "
        L"Split T/B/L %u/%u/%u | "
        L"Inv %u | "
        L"Tiles %u/%u | "
        L"Cells %u | "
        L"Cull %u | "
        L"Col %u->%u | "
        L"Range X[%d,%d] Y[%d,%d] | "
        L"Map %s | "
        L"VSync %s | "
        L"Target %s",

        stats.fps,
        stats.frameTimeMs,

        stats.engineCpuWorkMs,

        stats.fixedUpdateCpuMs,
        stats.fixedStepAverageCpuMs,
        stats.profiledFixedSteps,

        stats.updateCpuMs,
        stats.lateUpdateCpuMs,
        stats.renderCpuMs,
        stats.presentMs,

        stats.fixedUpdateHz,
        stats.fixedSteps,

        stats.entityCount,

        stats.renderCommands,

        stats.renderBatches,
        averageBatchSize,
        stats.maxBatchSize,

        stats.drawCalls,

        stats.textureBatchBoundaries,
        stats.blendBatchBoundaries,
        stats.layerBatchBoundaries,

        stats.invalidRenderCommands,

        stats.visibleTiles,
        stats.tileRenderItems,

        stats.tileCandidateCells,

        stats.culledTiles,

        stats.tileCollisionTiles,
        stats.tileCollisionShapes,

        stats.visibleTileMinX,
        stats.visibleTileMaxX,

        stats.visibleTileMinY,
        stats.visibleTileMaxY,

        stats.tileMapInView
        ? L"In"
        : L"Out",

        stats.vsync
        ? L"On"
        : L"Off",

        targetText
    );

    SetWindowTextW(
        m_window->GetHandle(),
        title
    );
}

Engine& Application::GetEngine()
{
    return *m_engine;
}

void Application::ProcessPendingResize()
{
    int width = 0;
    int height = 0;

    if (!m_window->ConsumeResize(
        width,
        height))
    {
        return;
    }

    if (width <= 0 ||
        height <= 0)
    {
        return;
    }

    m_engine->Resize(
        width,
        height
    );
}

void Application::SetTargetFPS(
    std::uint32_t fps)
{
    m_config.targetFPS =
        fps;
}

std::uint32_t
Application::GetTargetFPS() const
{
    return
        m_config.targetFPS;
}

void Application::SetVSync(
    bool enabled)
{
    m_config.vsync =
        enabled;
}

void Application::SetPaused(
    bool paused)
{
    if (m_isPaused == paused)
    {
        return;
    }

    m_isPaused =
        paused;

    if (m_isPaused)
    {
        m_prePauseTimeScale =
            Time::TimeScale();

        Time::SetTimeScale(
            0.0f
        );

        m_fixedAccumulator =
            0.0f;
    }
    else
    {
        float restoreScale =
            m_prePauseTimeScale;

        if (restoreScale <= 0.0f)
        {
            restoreScale = 1.0f;
        }

        Time::SetTimeScale(
            restoreScale
        );

        Time::ResetFrameTimer();
    }
}

bool Application::IsPaused() const
{
    return m_isPaused;
}

const EngineConfig&
Application::GetConfig() const
{
    return m_config;
}