#include "CpuProfiler.h"
#include <algorithm>


const wchar_t*
GetCpuProfileZoneLabel(
    CpuProfileZone zone) noexcept
{
    switch (zone)
    {
    case CpuProfileZone::FixedUpdate:
        return L"Fixed";

    case CpuProfileZone::Update:
        return L"Update";

    case CpuProfileZone::LateUpdate:
        return L"Late";

    case CpuProfileZone::RenderCpu:
        return L"Render";

    case CpuProfileZone::Present:
        return L"Present";

    case CpuProfileZone::SceneFixedUpdate:
        return L"SceneFixed";

    case CpuProfileZone::PhysicsStep:
        return L"Physics";

    case CpuProfileZone::PhysicsSync:
        return L"Sync";

    case CpuProfileZone::ContactDispatch:
        return L"Contact";

    case CpuProfileZone::RenderSubmit:
        return L"Submit";

    case CpuProfileZone::RenderSort:
        return L"Sort";

    case CpuProfileZone::RenderExecute:
        return L"Execute";

    case CpuProfileZone::DebugRender:
        return L"Debug";

    default:
        return L"None";
    }
}

namespace
{
    std::size_t ToIndex(
        CpuProfileZone zone
    ) noexcept
    {
        return
            static_cast<std::size_t>(
                zone
                );
    }

    bool IsValidZone(
        CpuProfileZone zone
    ) noexcept
    {
        return
            ToIndex(zone) <
            CpuProfileZoneCount;
    }
}



double CpuProfileSnapshot::GetTotalMs(
    CpuProfileZone zone) const noexcept
{
    if (!IsValidZone(zone))
    {
        return 0.0;
    }

    return
        totalMs[
            ToIndex(zone)
        ];
}


std::uint32_t
CpuProfileSnapshot::GetSampleCount(
    CpuProfileZone zone) const noexcept
{
    if (!IsValidZone(zone))
    {
        return 0;
    }

    return
        sampleCounts[
            ToIndex(zone)
        ];
}


double
CpuProfileSnapshot::GetAverageSampleMs(
    CpuProfileZone zone) const noexcept
{
    if (!IsValidZone(zone))
    {
        return 0.0;
    }

    const std::size_t index =
        ToIndex(zone);

    const std::uint32_t count =
        sampleCounts[index];

    if (count == 0)
    {
        return 0.0;
    }

    return
        totalMs[index] /
        static_cast<double>(
            count
            );
}


double
CpuProfileSnapshot::GetEngineCpuWorkMs()
const noexcept
{
    return
        GetTotalMs(
            CpuProfileZone::FixedUpdate
        ) +
        GetTotalMs(
            CpuProfileZone::Update
        ) +
        GetTotalMs(
            CpuProfileZone::LateUpdate
        ) +
        GetTotalMs(
            CpuProfileZone::RenderCpu
        );
}



void CpuProfiler::BeginFrame()
noexcept
{
    m_current =
        CpuProfileSnapshot{};

    m_frameOpen =
        true;
}

void CpuProfiler::EndFrame()
noexcept
{
    if (!m_frameOpen)
    {
        return;
    }

    m_latest =
        m_current;

    const double baselineAverageMs =
        CalculateHistoryAverageCpuWorkMs();

    const double currentCpuWorkMs =
        m_latest.GetEngineCpuWorkMs();


    double spikeThresholdMs =
        0.0;

    bool currentCpuSpike =
        false;


    if (m_historyCount >=
        SpikeWarmupFrames)
    {
        spikeThresholdMs =
            std::max(
                baselineAverageMs *
                SpikeRatioThreshold,

                baselineAverageMs +
                SpikeMinimumDeltaMs
            );

        currentCpuSpike =
            currentCpuWorkMs >
            spikeThresholdMs;
    }


    PushHistory(
        m_latest,
        currentCpuSpike
    );

    UpdateDiagnostics(
        spikeThresholdMs,
        currentCpuSpike
    );

    m_frameOpen =
        false;
}

void CpuProfiler::AddSample(
    CpuProfileZone zone,
    double elapsedMs) noexcept
{
    if (!m_frameOpen)
    {
        return;
    }

    if (!IsValidZone(zone))
    {
        return;
    }

    if (elapsedMs < 0.0)
    {
        return;
    }

    const std::size_t index =
        ToIndex(zone);

    m_current.totalMs[index] +=
        elapsedMs;

    ++m_current.sampleCounts[index];
}

bool CpuProfiler::IsFrameOpen()
const noexcept
{
    return m_frameOpen;
}

const CpuProfileSnapshot&
CpuProfiler::GetLatestSnapshot()
const noexcept
{
    return m_latest;
}

std::size_t
CpuProfiler::GetHistoryCount()
const noexcept
{
    return
        m_historyCount;
}

const CpuProfilerDiagnostics&
CpuProfiler::GetDiagnostics()
const noexcept
{
    return
        m_diagnostics;
}

const CpuProfileSnapshot*
CpuProfiler::GetHistorySnapshot(
    std::size_t framesAgo) const noexcept
{
    if (framesAgo >=
        m_historyCount)
    {
        return nullptr;
    }

    return
        &m_history[
            GetHistoryIndex(
                framesAgo
            )
        ];
}

void CpuProfiler::PushHistory(
    const CpuProfileSnapshot& snapshot,
    bool cpuSpike) noexcept
{
    m_history[
        m_historyWriteIndex
    ] =
        snapshot;

        m_spikeHistory[
            m_historyWriteIndex
        ] =
            cpuSpike;

            m_historyWriteIndex =
                (
                    m_historyWriteIndex +
                    1
                    ) %
                HistoryCapacity;

            if (m_historyCount <
                HistoryCapacity)
            {
                ++m_historyCount;
            }
}

void CpuProfiler::UpdateDiagnostics(
    double spikeThresholdMs,
    bool currentCpuSpike) noexcept
{
    m_diagnostics =
        CpuProfilerDiagnostics{};

    m_diagnostics.historyCount =
        static_cast<std::uint32_t>(
            m_historyCount
            );

    m_diagnostics.spikeThresholdMs =
        spikeThresholdMs;

    m_diagnostics.currentCpuSpike =
        currentCpuSpike;


    if (m_historyCount == 0)
    {
        return;
    }


    double totalCpuWorkMs =
        0.0;

    double totalPresentMs =
        0.0;

    double maxCpuWorkMs =
        0.0;

    double maxPresentMs =
        0.0;

    std::uint32_t maxCpuAge =
        InvalidCpuProfileFrameAge;

    std::uint32_t latestSpikeAge =
        InvalidCpuProfileFrameAge;

    std::uint32_t spikeCount =
        0;


    for (
        std::size_t age = 0;
        age < m_historyCount;
        ++age
        )
    {
        const std::size_t index =
            GetHistoryIndex(
                age
            );

        const CpuProfileSnapshot&
            snapshot =
            m_history[index];

        const double cpuWorkMs =
            snapshot.
            GetEngineCpuWorkMs();

        const double presentMs =
            snapshot.GetTotalMs(
                CpuProfileZone::Present
            );


        totalCpuWorkMs +=
            cpuWorkMs;

        totalPresentMs +=
            presentMs;


        if (age == 0 ||
            cpuWorkMs >
            maxCpuWorkMs)
        {
            maxCpuWorkMs =
                cpuWorkMs;

            maxCpuAge =
                static_cast<
                std::uint32_t
                >(age);
        }


        maxPresentMs =
            std::max(
                maxPresentMs,
                presentMs
            );


        if (m_spikeHistory[index])
        {
            ++spikeCount;

            if (latestSpikeAge ==
                InvalidCpuProfileFrameAge)
            {
                latestSpikeAge =
                    static_cast<
                    std::uint32_t
                    >(age);
            }
        }
    }


    const double historyCount =
        static_cast<double>(
            m_historyCount
            );


    m_diagnostics.
        averageEngineCpuWorkMs =
        totalCpuWorkMs /
        historyCount;

    m_diagnostics.
        maxEngineCpuWorkMs =
        maxCpuWorkMs;

    m_diagnostics.
        maxEngineCpuWorkFramesAgo =
        maxCpuAge;

    m_diagnostics.
        averagePresentMs =
        totalPresentMs /
        historyCount;

    m_diagnostics.
        maxPresentMs =
        maxPresentMs;

    m_diagnostics.
        cpuSpikesInHistory =
        spikeCount;

    m_diagnostics.
        latestSpikeFramesAgo =
        latestSpikeAge;

    if (maxCpuAge ==
        InvalidCpuProfileFrameAge)
    {
        return;
    }

    const CpuProfileSnapshot*
        peakSnapshot =
        GetHistorySnapshot(
            maxCpuAge
        );

    if (!peakSnapshot)
    {
        return;
    }


    constexpr
        std::array<
        CpuProfileZone,
        4
        >
        topLevelCpuZones
    {
        CpuProfileZone::FixedUpdate,
        CpuProfileZone::Update,
        CpuProfileZone::LateUpdate,
        CpuProfileZone::RenderCpu
    };


    for (CpuProfileZone zone :
    topLevelCpuZones)
    {
        const double value =
            peakSnapshot->
            GetTotalMs(
                zone
            );

        if (value >
            m_diagnostics.
            peakFrameWorstCpuPhaseMs)
        {
            m_diagnostics.
                peakFrameWorstCpuPhaseMs =
                value;

            m_diagnostics.
                peakFrameWorstCpuPhase =
                zone;
        }
    }


    constexpr
        std::array<
        CpuProfileZone,
        8
        >
        subsystemZones
    {
        CpuProfileZone::
            SceneFixedUpdate,

        CpuProfileZone::
            PhysicsStep,

        CpuProfileZone::
            PhysicsSync,

        CpuProfileZone::
            ContactDispatch,

        CpuProfileZone::
            RenderSubmit,

        CpuProfileZone::
            RenderSort,

        CpuProfileZone::
            RenderExecute,

        CpuProfileZone::
            DebugRender
    };


    for (CpuProfileZone zone :
    subsystemZones)
    {
        const double value =
            peakSnapshot->
            GetTotalMs(
                zone
            );

        if (value >
            m_diagnostics.
            peakFrameWorstSubsystemMs)
        {
            m_diagnostics.
                peakFrameWorstSubsystemMs =
                value;

            m_diagnostics.
                peakFrameWorstSubsystem =
                zone;
        }
    }
}

std::size_t
CpuProfiler::GetHistoryIndex(
    std::size_t framesAgo) const noexcept
{
    const std::size_t latestIndex =
        (
            m_historyWriteIndex +
            HistoryCapacity -
            1
            ) %
        HistoryCapacity;

    return
        (
            latestIndex +
            HistoryCapacity -
            framesAgo
            ) %
        HistoryCapacity;
}

double
CpuProfiler::
CalculateHistoryAverageCpuWorkMs()
const noexcept
{
    if (m_historyCount == 0)
    {
        return 0.0;
    }

    double totalMs =
        0.0;

    for (
        std::size_t age = 0;
        age < m_historyCount;
        ++age
        )
    {
        const CpuProfileSnapshot*
            snapshot =
            GetHistorySnapshot(
                age
            );

        if (!snapshot)
        {
            continue;
        }

        totalMs +=
            snapshot->
            GetEngineCpuWorkMs();
    }

    return
        totalMs /
        static_cast<double>(
            m_historyCount
            );
}


ScopedCpuProfile::ScopedCpuProfile(
    CpuProfiler& profiler,
    CpuProfileZone zone) noexcept
    :
    m_profiler(
        profiler.IsFrameOpen()
        ? &profiler
        : nullptr
    ),
    m_zone(zone)
{
    if (!m_profiler)
    {
        return;
    }

    m_start =
        Clock::now();
}


ScopedCpuProfile::~ScopedCpuProfile()
{
    if (!m_profiler)
    {
        return;
    }

    const Clock::time_point end =
        Clock::now();

    const double elapsedMs =
        std::chrono::duration<
        double,
        std::milli
        >(
            end - m_start
        ).count();

    m_profiler->AddSample(
        m_zone,
        elapsedMs
    );
}
