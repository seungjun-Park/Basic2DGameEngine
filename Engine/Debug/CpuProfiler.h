#pragma once


#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits.h>

enum class CpuProfileZone :
    std::uint8_t
{
    //
    // Top-level frame phases.
    //
    FixedUpdate = 0,
    Update,
    LateUpdate,
    RenderCpu,
    Present,

    //
    // FixedUpdate children.
    //
    SceneFixedUpdate,
    PhysicsStep,
    PhysicsSync,
    ContactDispatch,

    //
    // RenderCpu children.
    //
    RenderSubmit,
    RenderSort,
    RenderExecute,
    DebugRender,

    Count
};


constexpr std::size_t
CpuProfileZoneCount =
static_cast<std::size_t>(
    CpuProfileZone::Count
    );

constexpr std::uint32_t InvalidCpuProfileFrameAge = std::numeric_limits<std::uint32_t>::max();


struct CpuProfileSnapshot
{
    std::array<
        double,
        CpuProfileZoneCount
    >
        totalMs{};

    std::array<
        std::uint32_t,
        CpuProfileZoneCount
    >
        sampleCounts{};


    double GetTotalMs(
        CpuProfileZone zone
    ) const noexcept;

    std::uint32_t GetSampleCount(
        CpuProfileZone zone
    ) const noexcept;

    double GetAverageSampleMs(
        CpuProfileZone zone
    ) const noexcept;

    //
    // Present는 포함하지 않는다.
    //
    // 즉 Engine이 frame 안에서 수행한
    // 실제 CPU work 합계.
    //
    double GetEngineCpuWorkMs()
        const noexcept;
};


struct CpuProfilerDiagnostics
{
    std::uint32_t
        historyCount = 0;

    //
    // 최근 history의 CPU work 평균.
    //
    double
        averageEngineCpuWorkMs = 0.0;

    //
    // 최근 history 중 가장 비쌌던 CPU frame.
    //
    double
        maxEngineCpuWorkMs = 0.0;

    //
    // max CPU frame이 현재로부터 몇 frame 전인지.
    //
    std::uint32_t
        maxEngineCpuWorkFramesAgo =
        InvalidCpuProfileFrameAge;


    //
    // Present는 CPU spike 판정에서 제외하지만
    // 별도 diagnostics로 유지.
    //
    double
        averagePresentMs = 0.0;

    double
        maxPresentMs = 0.0;


    //
    // 이번 frame을 판정할 때 사용한 threshold.
    //
    // warm-up 중에는 0.
    //
    double
        spikeThresholdMs = 0.0;

    bool
        currentCpuSpike = false;


    //
    // 현재 history window 안에 남아 있는
    // spike frame 수.
    //
    std::uint32_t
        cpuSpikesInHistory = 0;

    //
    // 가장 최근 spike가 몇 frame 전인지.
    //
    std::uint32_t
        latestSpikeFramesAgo =
        InvalidCpuProfileFrameAge;


    //
    // History에서 CPU work가 최대였던 frame의
    // 가장 비싼 top-level phase.
    //
    CpuProfileZone
        peakFrameWorstCpuPhase =
        CpuProfileZone::Count;

    double
        peakFrameWorstCpuPhaseMs =
        0.0;


    //
    // 같은 peak frame 내부의
    // 가장 비싼 subsystem child zone.
    //
    CpuProfileZone
        peakFrameWorstSubsystem =
        CpuProfileZone::Count;

    double
        peakFrameWorstSubsystemMs =
        0.0;
};

class CpuProfiler final
{
public:

    void BeginFrame() noexcept;

    void EndFrame() noexcept;

    void AddSample(
        CpuProfileZone zone,
        double elapsedMs
    ) noexcept;

    bool IsFrameOpen()
        const noexcept;

    const CpuProfileSnapshot&
        GetLatestSnapshot()
        const noexcept;

    const CpuProfilerDiagnostics&
        GetDiagnostics()
        const noexcept;

    std::size_t GetHistoryCount()
        const noexcept;

    //
    // framesAgo:
    //
    // 0 = latest completed frame
    // 1 = previous frame
    // ...
    //
    // history 범위를 벗어나면 nullptr.
    //
    const CpuProfileSnapshot*
        GetHistorySnapshot(
            std::size_t framesAgo
        ) const noexcept;

private:

    std::size_t GetHistoryIndex(
        std::size_t framesAgo
    ) const noexcept;

    double
        CalculateHistoryAverageCpuWorkMs()
        const noexcept;

    void PushHistory(
        const CpuProfileSnapshot& snapshot,
        bool cpuSpike
    ) noexcept;

    void UpdateDiagnostics(
        double spikeThresholdMs,
        bool currentCpuSpike
    ) noexcept;

public:

    static constexpr
        std::size_t
        HistoryCapacity = 120;

    static constexpr
        std::size_t
        SpikeWarmupFrames = 30;

    static constexpr
        double
        SpikeRatioThreshold = 2.0;

    static constexpr
        double
        SpikeMinimumDeltaMs = 1.0;
private:

    CpuProfileSnapshot
        m_current{};

    CpuProfileSnapshot
        m_latest{};

    bool
        m_frameOpen = false;

    std::array<
        CpuProfileSnapshot,
        HistoryCapacity
    >
        m_history{};

    std::array<
        bool,
        HistoryCapacity
    >
        m_spikeHistory{};

    std::size_t
        m_historyWriteIndex = 0;

    std::size_t
        m_historyCount = 0;

    CpuProfilerDiagnostics
        m_diagnostics{};
};


class ScopedCpuProfile final
{
public:

    ScopedCpuProfile(
        CpuProfiler& profiler,
        CpuProfileZone zone
    ) noexcept;

    ~ScopedCpuProfile();


    ScopedCpuProfile(
        const ScopedCpuProfile&
    ) = delete;

    ScopedCpuProfile&
        operator=(
            const ScopedCpuProfile&
            ) = delete;

    ScopedCpuProfile(
        ScopedCpuProfile&&
    ) = delete;

    ScopedCpuProfile&
        operator=(
            ScopedCpuProfile&&
            ) = delete;


private:

    using Clock =
        std::chrono::steady_clock;

    CpuProfiler*
        m_profiler = nullptr;

    CpuProfileZone
        m_zone =
        CpuProfileZone::Update;

    Clock::time_point
        m_start{};
};

const wchar_t*
GetCpuProfileZoneLabel(
    CpuProfileZone zone
) noexcept;