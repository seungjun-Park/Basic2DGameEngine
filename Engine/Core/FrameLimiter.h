#pragma once

#include <cstdint>

class FrameLimiter
{
public:
    FrameLimiter();
    ~FrameLimiter();

    FrameLimiter(
        const FrameLimiter&) = delete;

    FrameLimiter& operator=(
        const FrameLimiter&) = delete;

    void BeginFrame();

    void EndFrame(
        std::uint32_t targetFPS
    );

private:
    double GetTimestampSeconds() const;

private:
    double m_frequency = 0.0;
    double m_frameStartTime = 0.0;

    void* m_waitableTimer = nullptr;
};