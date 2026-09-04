#pragma once

#include <cstddef>

class AnimationClip;
struct Sprite;

class Animator
{
public:
    explicit Animator(
        Sprite& sprite
    ) noexcept;

    void Update(
        float deltaTime
    );

    bool Play(
        const AnimationClip& clip,
        bool restart = false
    );

    void Stop() noexcept;

    bool IsPlaying() const noexcept
    {
        return m_playing;
    }

    std::size_t GetCurrentFrameIndex() const noexcept
    {
        return m_frameIndex;
    }

    const AnimationClip* GetCurrentClip() const noexcept
    {
        return m_clip;
    }

private:
    void ApplyCurrentFrame();

private:

    Sprite* m_sprite = nullptr;

    const AnimationClip* m_clip = nullptr;

    std::size_t m_frameIndex = 0;
    float m_frameElapsed = 0.0f;
    bool m_playing = false;
};