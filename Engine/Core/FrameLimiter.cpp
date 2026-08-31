#include "FrameLimiter.h"

#include <Windows.h>

#include <chrono>
#include <thread>

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
            ) / m_frequency;
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

    while (true)
    {
        const double elapsed =
            GetTimestampSeconds() -
            m_frameStartTime;

        const double remaining =
            targetFrameTime -
            elapsed;

        if (remaining <= 0.0)
        {
            break;
        }

        // 충분한 시간이 남았으면
        // CPU를 잠시 OS에 양보
        if (remaining > 0.002)
        {
            const double sleepTime =
                remaining - 0.001;

            std::this_thread::sleep_for(
                std::chrono::duration<double>(
                    sleepTime
                )
            );
        }
        else
        {
            // 마지막 짧은 구간은
            // coarse한 Sleep 대신 yield
            std::this_thread::yield();
        }
    }
}