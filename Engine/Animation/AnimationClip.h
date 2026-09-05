#pragma once

#include "Engine/Graphics/UVRect.h"

#include <cstddef>
#include <cstdint>
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
        if (m_texture == texture)
        {
            return;
        }

        m_texture = texture;

        ++m_revision;
    }

    void SetLooping(
        bool looping
    ) noexcept
    {
        if (m_looping == looping)
        {
            return;
        }

        m_looping = looping;

        ++m_revision;
    }

    bool AddFrame(
        const UVRect& uv,
        float duration
    );

    void ClearFrames();

    bool ReplaceContents(
        Texture* texture,
        bool looping,
        std::vector<AnimationFrame> frames
    );

    [[nodiscard]]
    std::uint64_t GetRevision() const noexcept
    {
        return m_revision;
    }


    Texture* GetTexture() const noexcept
    {
        return m_texture;
    }

    std::size_t GetFrameCount() const noexcept
    {
        return m_frames.size();
    }

    const AnimationFrame* GetFrame(
        std::size_t index
    ) const noexcept;

    bool IsValid() const noexcept;
    bool IsLooping() const noexcept
    {
        return m_looping;
    }


private:
    std::vector<AnimationFrame> m_frames;

    Texture* m_texture = nullptr;

    std::uint64_t m_revision = 0;

    bool m_looping = true;
};