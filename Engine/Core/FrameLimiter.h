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

    // targetFPS == 0이면 Unlimited
    void EndFrame(
        std::uint32_t targetFPS
    );

private:
    double GetTimestampSeconds() const;

private:
    double m_frequency = 0.0;
    double m_frameStartTime = 0.0;

    //
    // Windows waitable timer HANDLE.
    //
    // Header에 Windows.h 의존성을 퍼뜨리지 않기 위해
    // opaque pointer로 보관한다.
    //
    void* m_waitableTimer = nullptr;
};