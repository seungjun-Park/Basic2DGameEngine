#pragma once

class Time
{
public:
    static void Initialize();
    static void Tick();

    static float DeltaTime();
    static float TotalTime();
    static float FPS();

private:
    static double s_frequency;
    static long long s_previousCounter;

    static float s_deltaTime;
    static float s_totalTime;
    static float s_fps;

    static int s_frameCount;
    static float s_fpsTimer;
};