#pragma once

class Time
{
public:
    static void Initialize(
        float fixedUpdateHz,
        float maxDeltaTime
    );

    static void Tick();

    // TimeScale 적용 시간
    static float DeltaTime();

    // 실제 frame 시간
    static float UnscaledDeltaTime();

    static float FixedDeltaTime();

    static float TotalTime();
    static float UnscaledTotalTime();

    static float FPS();

    static float TimeScale();

    static void SetTimeScale(
        float scale
    );

    // Alt+Tab / 최소화 복귀 시
    // 큰 DeltaTime이 발생하지 않도록 기준 시간 재설정
    static void ResetFrameTimer();

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