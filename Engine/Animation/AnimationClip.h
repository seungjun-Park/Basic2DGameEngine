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

class AnimationClip final
{
public:
    AnimationClip() = default;
    ~AnimationClip() = default;

    void SetTexture(
        Texture* texture) noexcept
    {
        m_texture = texture;
    }

    void SetLooping(
        bool looping) noexcept
    {
        m_looping = looping;
    }
    
    bool AddFrame(
        const UVRect& uv,
        float duration
    );

    bool IsLooping() const noexcept
    {
        return m_looping;
    }

    bool IsValid() const noexcept;


    void ClearFrames();
    
    Texture* GetTexture() const noexcept
    {
        return m_texture;
    }


    std::size_t
        GetFrameCount() const noexcept
    {
        return m_frames.size();
    }

    const AnimationFrame*
        GetFrame(
            std::size_t index
        ) const noexcept;

private:
    Texture* m_texture = nullptr;
    std::vector<AnimationFrame> m_frames;
    bool m_looping = true;
};