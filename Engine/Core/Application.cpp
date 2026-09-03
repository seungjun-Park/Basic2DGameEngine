#include "Application.h"

#include "Engine.h"
#include "Time.h"

#include "Engine/Platform/Windows/WinWindow.h"
#include "Engine/Platform/Windows/WinInput.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Debug/CpuProfiler.h"

#include <algorithm>
#include <cstdint>
#include <cwchar>

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

        m_engine->BeginGuiFrame();

        WinInput::Update();

        if (WinInput::IsRawKeyPressed(VK_F2))
        {
            m_showProfilerTitle =
                !m_showProfilerTitle;
        }

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
            std::min(
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

        ReportProfilerSpikeIfNeeded();

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

    wchar_t title[1536]{};

    const int peakFrameAge =
        stats.cpuWorkMaxFramesAgo ==
        InvalidCpuProfileFrameAge
        ?
        -1
        :
        static_cast<int>(
            stats.cpuWorkMaxFramesAgo
            );

    const int latestSpikeAge =
        stats.latestCpuSpikeFramesAgo ==
        InvalidCpuProfileFrameAge
        ?
        -1
        :
        static_cast<int>(
            stats.latestCpuSpikeFramesAgo
            );

    const wchar_t*
        peakPhaseLabel =
        GetCpuProfileZoneLabel(
            stats.peakFrameWorstCpuPhase
        );

    const wchar_t*
        peakSubsystemLabel =
        GetCpuProfileZoneLabel(
            stats.peakFrameWorstSubsystem
        );

    if (m_showProfilerTitle)
    {
        const int peakFrameAge =
            stats.cpuWorkMaxFramesAgo ==
            InvalidCpuProfileFrameAge
            ?
            -1
            :
            static_cast<int>(
                stats.cpuWorkMaxFramesAgo
                );

        const int latestSpikeAge =
            stats.latestCpuSpikeFramesAgo ==
            InvalidCpuProfileFrameAge
            ?
            -1
            :
            static_cast<int>(
                stats.latestCpuSpikeFramesAgo
                );


        const wchar_t*
            peakPhaseLabel =
            GetCpuProfileZoneLabel(
                stats.peakFrameWorstCpuPhase
            );

        const wchar_t*
            peakSubsystemLabel =
            GetCpuProfileZoneLabel(
                stats.peakFrameWorstSubsystem
            );


        wchar_t title[1024]{};

        swprintf_s(
            title,

            L"Dobi2D [Profiler:F2] | "
            L"FPS %.1f | "
            L"Frame %.2f ms | "

            L"CPU %.2f "
            L"Avg %.2f "
            L"Max %.2f@%df | "

            L"F %.2f/%u | "
            L"U %.2f | "
            L"L %.2f | "
            L"R %.2f | "
            L"P %.2f | "

            L"Spk %u "
            L"last %df "
            L"thr %.2f | "

            L"Peak %ls %.2f / "
            L"%ls %.2f",

            stats.fps,
            stats.frameTimeMs,

            stats.engineCpuWorkMs,
            stats.cpuWorkAverageMs,
            stats.cpuWorkMaxMs,
            peakFrameAge,

            stats.fixedUpdateCpuMs,
            stats.profiledFixedSteps,

            stats.updateCpuMs,
            stats.lateUpdateCpuMs,
            stats.renderCpuMs,
            stats.presentMs,

            stats.cpuSpikesInHistory,
            latestSpikeAge,
            stats.cpuSpikeThresholdMs,

            peakPhaseLabel,
            stats.peakFrameWorstCpuPhaseMs,

            peakSubsystemLabel,
            stats.peakFrameWorstSubsystemMs
        );


        ::SetWindowTextW(
            m_window->GetHandle(),
            title
        );

        return;
    }

    swprintf_s(
        title,

        L"Demo | "
        L"FPS %.1f | "
        L"Frame %.2f ms | "
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

void Application::
ReportProfilerSpikeIfNeeded()
{
#if defined(_DEBUG)

    if (!m_engine)
    {
        return;
    }


    const DebugStats& stats =
        m_engine->GetDebugStats();


    if (!stats.currentCpuSpike)
    {
        return;
    }


    CpuProfileZone
        worstPhase =
        CpuProfileZone::Count;

    float worstPhaseMs =
        0.0f;


    auto considerPhase =
        [&](
            CpuProfileZone zone,
            float value)
        {
            if (value <=
                worstPhaseMs)
            {
                return;
            }

            worstPhase =
                zone;

            worstPhaseMs =
                value;
        };


    considerPhase(
        CpuProfileZone::FixedUpdate,
        stats.fixedUpdateCpuMs
    );

    considerPhase(
        CpuProfileZone::Update,
        stats.updateCpuMs
    );

    considerPhase(
        CpuProfileZone::LateUpdate,
        stats.lateUpdateCpuMs
    );

    considerPhase(
        CpuProfileZone::RenderCpu,
        stats.renderCpuMs
    );


    CpuProfileZone
        worstSubsystem =
        CpuProfileZone::Count;

    float worstSubsystemMs =
        0.0f;


    auto considerSubsystem =
        [&](
            CpuProfileZone zone,
            float value)
        {
            if (value <=
                worstSubsystemMs)
            {
                return;
            }

            worstSubsystem =
                zone;

            worstSubsystemMs =
                value;
        };


    considerSubsystem(
        CpuProfileZone::
        SceneFixedUpdate,
        stats.sceneFixedCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        PhysicsStep,
        stats.physicsStepCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        PhysicsSync,
        stats.physicsSyncCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        ContactDispatch,
        stats.contactDispatchCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        RenderSubmit,
        stats.renderSubmitCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        RenderSort,
        stats.renderSortCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        RenderExecute,
        stats.renderExecuteCpuMs
    );

    considerSubsystem(
        CpuProfileZone::
        DebugRender,
        stats.debugRenderCpuMs
    );


    wchar_t message[1536]{};

    swprintf_s(
        message,

        L"[Profiler] CPU SPIKE | "
        L"CPU %.2f ms | "
        L"WindowAvg %.2f ms | "
        L"Threshold %.2f ms | "
        L"Frame %.2f ms | "

        L"Fixed %.2f | "
        L"Update %.2f | "
        L"Late %.2f | "
        L"Render %.2f | "
        L"Present %.2f | "

        L"Worst %ls %.2f | "
        L"Subsystem %ls %.2f | "

        L"History %u | "
        L"Spikes %u\n",

        stats.engineCpuWorkMs,
        stats.cpuWorkAverageMs,
        stats.cpuSpikeThresholdMs,
        stats.frameTimeMs,

        stats.fixedUpdateCpuMs,
        stats.updateCpuMs,
        stats.lateUpdateCpuMs,
        stats.renderCpuMs,
        stats.presentMs,

        GetCpuProfileZoneLabel(
            worstPhase
        ),
        worstPhaseMs,

        GetCpuProfileZoneLabel(
            worstSubsystem
        ),
        worstSubsystemMs,

        stats.profilerHistoryFrames,
        stats.cpuSpikesInHistory
    );


    ::OutputDebugStringW(
        message
    );

#endif
}