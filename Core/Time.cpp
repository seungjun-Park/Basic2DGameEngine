#include "Time.h"

#include <Windows.h>
#include <algorithm>
#include <math.h>

double Time::s_frequency = 0.0;
long long Time::s_previousCounter = 0;

float Time::s_deltaTime = 0.0f;
float Time::s_totalTime = 0.0f;
float Time::s_fps = 0.0f;

int Time::s_frameCount = 0;
float Time::s_fpsTimer = 0.0f;

void Time::Initialize()
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;

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

    s_deltaTime = 0.0f;
    s_totalTime = 0.0f;
    s_fps = 0.0f;

    s_frameCount = 0;
    s_fpsTimer = 0.0f;
}

void Time::Tick()
{
    LARGE_INTEGER counter;

    QueryPerformanceCounter(
        &counter
    );

    long long elapsed =
        counter.QuadPart -
        s_previousCounter;

    s_previousCounter =
        counter.QuadPart;

    s_deltaTime =
        static_cast<float>(
            static_cast<double>(elapsed) /
            s_frequency
            );

    // 디버거에서 breakpoint를 걸었다가
    // 돌아왔을 때 비정상적으로 큰 dt가
    // 게임 로직에 들어가는 것을 방지한다.
    s_deltaTime = min(s_deltaTime, 0.1f);

    s_totalTime += s_deltaTime;

    ++s_frameCount;

    s_fpsTimer += s_deltaTime;

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

float Time::TotalTime()
{
    return s_totalTime;
}

float Time::FPS()
{
    return s_fps;
}