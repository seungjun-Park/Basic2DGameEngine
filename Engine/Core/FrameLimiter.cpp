#include "FrameLimiter.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace
{
    constexpr double
        HundredNanosecondsPerSecond =
        10'000'000.0;

    constexpr DWORD
        HighResolutionTimerFlag =
        0x00000002;

    constexpr double
        MaxSpinSeconds =
        0.0002; // 0.2 ms
}

FrameLimiter::FrameLimiter()
{
    LARGE_INTEGER frequency{};

    QueryPerformanceFrequency(
        &frequency
    );

    m_frequency =
        static_cast<double>(
            frequency.QuadPart
            );

    HANDLE timer =
        CreateWaitableTimerExW(
            nullptr,
            nullptr,
            HighResolutionTimerFlag,
            TIMER_MODIFY_STATE |
            SYNCHRONIZE
        );

    if (!timer)
    {
        timer =
            CreateWaitableTimerW(
                nullptr,
                FALSE,
                nullptr
            );
    }

    m_waitableTimer =
        timer;
}

FrameLimiter::~FrameLimiter()
{
    if (m_waitableTimer)
    {
        CloseHandle(
            static_cast<HANDLE>(
                m_waitableTimer
                )
        );

        m_waitableTimer =
            nullptr;
    }
}

void FrameLimiter::BeginFrame()
{
    m_frameStartTime =
        GetTimestampSeconds();
}

void FrameLimiter::EndFrame(
    std::uint32_t targetFPS)
{
    if (targetFPS == 0)
    {
        return;
    }

    const double targetFrameTime =
        1.0 /
        static_cast<double>(
            targetFPS
            );

    const double targetEndTime =
        m_frameStartTime +
        targetFrameTime;

    const double spinSeconds =
        std::min(
            MaxSpinSeconds,
            targetFrameTime * 0.25
        );

    while (true)
    {
        const double now =
            GetTimestampSeconds();

        const double remaining =
            targetEndTime - now;

        if (remaining <= 0.0)
        {
            break;
        }

        if (remaining > spinSeconds)
        {
            const double waitSeconds =
                remaining -
                spinSeconds;

            HANDLE timer =
                static_cast<HANDLE>(
                    m_waitableTimer
                    );

            if (timer)
            {
                LARGE_INTEGER dueTime{};

                auto dueTicks =
                    static_cast<LONGLONG>(
                        waitSeconds *
                        HundredNanosecondsPerSecond
                        );

                dueTicks =
                    std::max<LONGLONG>(
                        dueTicks,
                        1
                    );

                dueTime.QuadPart =
                    -dueTicks;

                if (SetWaitableTimer(
                    timer,
                    &dueTime,
                    0,
                    nullptr,
                    nullptr,
                    FALSE))
                {
                    WaitForSingleObject(
                        timer,
                        INFINITE
                    );

                    continue;
                }
            }

            std::this_thread::sleep_for(
                std::chrono::duration<double>(
                    waitSeconds
                )
            );

            continue;
        }

        YieldProcessor();
    }
}

double FrameLimiter::GetTimestampSeconds() const
{
    LARGE_INTEGER counter{};

    QueryPerformanceCounter(
        &counter
    );

    return
        static_cast<double>(
            counter.QuadPart
            ) /
        m_frequency;
}