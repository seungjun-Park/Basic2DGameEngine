#pragma once


#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits.h>

enum class CpuProfileZone :
    std::uint8_t
{
    FixedUpdate = 0,
    Update,
    LateUpdate,
    RenderCpu,
    Present,

    SceneFixedUpdate,
    PhysicsStep,
    PhysicsSync,
    ContactDispatch,

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

    double GetEngineCpuWorkMs()
        const noexcept;
};


struct CpuProfilerDiagnostics
{
    std::uint32_t
        historyCount = 0;

    double
        averageEngineCpuWorkMs = 0.0;

    double
        maxEngineCpuWorkMs = 0.0;

    std::uint32_t
        maxEngineCpuWorkFramesAgo =
        InvalidCpuProfileFrameAge;

    double
        averagePresentMs = 0.0;

    double
        maxPresentMs = 0.0;

    double
        spikeThresholdMs = 0.0;

    bool
        currentCpuSpike = false;

    std::uint32_t
        cpuSpikesInHistory = 0;

    std::uint32_t
        latestSpikeFramesAgo =
        InvalidCpuProfileFrameAge;

    CpuProfileZone
        peakFrameWorstCpuPhase =
        CpuProfileZone::Count;

    double
        peakFrameWorstCpuPhaseMs =
        0.0;

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

    const CpuProfileSnapshot*
        GetHistorySnapshot(
            std::size_t framesAgo
        ) const noexcept;

private:
    void PushHistory(
        const CpuProfileSnapshot& snapshot,
        bool cpuSpike
    ) noexcept;

    void UpdateDiagnostics(
        double spikeThresholdMs,
        bool currentCpuSpike
    ) noexcept;

    std::size_t GetHistoryIndex(
        std::size_t framesAgo
    ) const noexcept;

    double
        CalculateHistoryAverageCpuWorkMs()
        const noexcept;

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

    ScopedCpuProfile(
        const ScopedCpuProfile&
    ) = delete;

    ScopedCpuProfile(
        ScopedCpuProfile&&
    ) = delete;


    ~ScopedCpuProfile();

    ScopedCpuProfile&
        operator=(
            const ScopedCpuProfile&
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