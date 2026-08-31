#include "Time.h"

#include <Windows.h>

#include <algorithm>

double Time::s_frequency = 0.0;

long long Time::s_previousCounter = 0;

float Time::s_deltaTime = 0.0f;
float Time::s_unscaledDeltaTime = 0.0f;

float Time::s_fixedDeltaTime =
1.0f / 60.0f;

float Time::s_totalTime = 0.0f;
float Time::s_unscaledTotalTime = 0.0f;

float Time::s_timeScale = 1.0f;

float Time::s_maxDeltaTime = 0.1f;

float Time::s_fps = 0.0f;

int Time::s_frameCount = 0;
float Time::s_fpsTimer = 0.0f;

void Time::Initialize(
    float fixedUpdateHz,
    float maxDeltaTime)
{
    LARGE_INTEGER frequency{};
    LARGE_INTEGER counter{};

    QueryPerformanceFrequency(
        &frequency
    );

    QueryPerformanceCounter(
        &counter
    );

    s_frequency =
        static_cast<double>(
            frequency.QuadPart
            );

    s_previousCounter =
        counter.QuadPart;

    fixedUpdateHz =
        max(
            fixedUpdateHz,
            1.0f
        );

    s_fixedDeltaTime =
        1.0f /
        fixedUpdateHz;

    s_maxDeltaTime =
        max(
            maxDeltaTime,
            0.001f
        );

    s_deltaTime = 0.0f;
    s_unscaledDeltaTime = 0.0f;

    s_totalTime = 0.0f;
    s_unscaledTotalTime = 0.0f;

    s_timeScale = 1.0f;

    s_fps = 0.0f;

    s_frameCount = 0;
    s_fpsTimer = 0.0f;
}

void Time::Tick()
{
    LARGE_INTEGER counter{};

    QueryPerformanceCounter(
        &counter
    );

    const long long elapsedCounter =
        counter.QuadPart -
        s_previousCounter;

    s_previousCounter =
        counter.QuadPart;

    const double elapsedSeconds =
        static_cast<double>(
            elapsedCounter
            ) / s_frequency;

    s_unscaledDeltaTime =
        static_cast<float>(
            elapsedSeconds
            );

    // Breakpoint / Alt+Tab 등에 의해
    // 지나치게 큰 DeltaTime이 들어가는 것 방지
    s_unscaledDeltaTime =
        min(
            s_unscaledDeltaTime,
            s_maxDeltaTime
        );

    s_deltaTime =
        s_unscaledDeltaTime *
        s_timeScale;

    s_unscaledTotalTime +=
        s_unscaledDeltaTime;

    s_totalTime +=
        s_deltaTime;

    ++s_frameCount;

    s_fpsTimer +=
        s_unscaledDeltaTime;

    if (s_fpsTimer >= 1.0f)
    {
        s_fps =
            static_cast<float>(
                s_frameCount
                ) / s_fpsTimer;

        s_frameCount = 0;
        s_fpsTimer = 0.0f;
    }
}

float Time::DeltaTime()
{
    return s_deltaTime;
}

float Time::UnscaledDeltaTime()
{
    return s_unscaledDeltaTime;
}

float Time::FixedDeltaTime()
{
    return s_fixedDeltaTime;
}

float Time::TotalTime()
{
    return s_totalTime;
}

float Time::UnscaledTotalTime()
{
    return s_unscaledTotalTime;
}

float Time::FPS()
{
    return s_fps;
}

float Time::TimeScale()
{
    return s_timeScale;
}

void Time::SetTimeScale(
    float scale)
{
    s_timeScale =
        max(
            scale,
            0.0f
        );
}

void Time::ResetFrameTimer()
{
    LARGE_INTEGER counter{};

    QueryPerformanceCounter(
        &counter
    );

    s_previousCounter =
        counter.QuadPart;

    s_deltaTime = 0.0f;
    s_unscaledDeltaTime = 0.0f;
}