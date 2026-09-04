#pragma once

#include "Engine/Graphics/UVRect.h"

#include <cstddef>
#include <vector>

class Texture;

struct AnimationFrame
{
    UVRect uv;

    float duration =
        0.1f;
};

class AnimationClip
{
public:
    void SetTexture(
        Texture* texture
    ) noexcept
    {
        m_texture = texture;
    }

    void SetLooping(
        bool looping
    ) noexcept
    {
        m_looping = looping;
    }

    bool AddFrame(
        const UVRect& uv,
        float duration
    );

    void ClearFrames();

    Texture* GetTexture() const noexcept
    {
        return m_texture;
    }

    bool IsLooping() const noexcept
    {
        return m_looping;
    }

    std::size_t GetFrameCount() const noexcept
    {
        return m_frames.size();
    }

    const AnimationFrame* GetFrame(
        std::size_t index
    ) const noexcept;

    bool IsValid() const noexcept;

private:
    std::vector<AnimationFrame> m_frames;

    Texture* m_texture = nullptr;

    bool m_looping = true;
};