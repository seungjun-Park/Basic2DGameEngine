#pragma once

#include <cstdint>

class FrameLimiter
{
public:
    FrameLimiter();

    void BeginFrame();

    // targetFPS == 0ÀÌ¸י Unlimited
    void EndFrame(
        std::uint32_t targetFPS
    );

private:
    double GetTimestampSeconds() const;

private:
    double m_frequency = 0.0;
    double m_frameStartTime = 0.0;

    bool m_timerResolutionEnabled = false;
};
