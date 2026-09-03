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

    //
    // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
    //
    // Windows 10 version 1803+
    //
    constexpr DWORD
        HighResolutionTimerFlag =
        0x00000002;

    //
    // 마지막 구간은 OS wait를 사용하지 않고
    // 짧게 spin하여 scheduler overshoot를 줄인다.
    //
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

    //
    // 우선 Windows 10 1803+의
    // high-resolution waitable timer를 요청한다.
    //
    HANDLE timer =
        CreateWaitableTimerExW(
            nullptr,
            nullptr,
            HighResolutionTimerFlag,
            TIMER_MODIFY_STATE |
            SYNCHRONIZE
        );

    //
    // 지원하지 않는 Windows에서는
    // 일반 waitable timer로 fallback.
    //
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

    //
    // 매우 높은 target FPS에서는
    // 전체 frame의 상당 부분을 spin하지 않도록
    // spin 구간을 frame time의 최대 25%로 제한한다.
    //
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

        //
        // 충분한 시간이 남아 있다면
        // CPU를 OS에 양보한다.
        //
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

                //
                // negative value =
                // relative expiration time
                //
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

            //
            // Waitable timer 생성/설정 실패 시
            // 기존 방식으로 fallback.
            //
            std::this_thread::sleep_for(
                std::chrono::duration<double>(
                    waitSeconds
                )
            );

            continue;
        }

        //
        // 마지막 매우 짧은 구간.
        //
        // yield()는 thread가 다시 scheduling될 시점을
        // 보장하지 않으므로 frame pacing에 부적합하다.
        //
        // YieldProcessor()는 x86/x64에서 pause hint로
        // 동작하며 짧은 spin loop의 부하를 줄인다.
        //
        YieldProcessor();
    }
}