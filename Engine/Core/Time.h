#pragma once

class Time
{
public:
    static void Initialize(
        float fixedUpdateHz,
        float maxDeltaTime
    );

    static void Tick();
    static void ResetFrameTimer();

    static void SetTimeScale(
        float scale
    );
    static float DeltaTime();
    static float UnscaledDeltaTime();
    static float FixedDeltaTime();
    static float TotalTime();
    static float UnscaledTotalTime();
    static float FPS();
    static float TimeScale();

private:
    static double s_frequency;

    static long long
        s_previousCounter;

    static float s_deltaTime;
    static float s_unscaledDeltaTime;

    static float s_fixedDeltaTime;

    static float s_totalTime;
    static float s_unscaledTotalTime;

    static float s_timeScale;
    static float s_maxDeltaTime;

    static float s_fps;

    static int s_frameCount;
    static float s_fpsTimer;
};