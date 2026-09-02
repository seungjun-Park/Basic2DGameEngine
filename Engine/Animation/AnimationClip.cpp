#include "AnimationClip.h"

#include <cmath>

namespace
{
    bool IsValidUVRect(
        const UVRect& uv) noexcept
    {
        if (!std::isfinite(uv.u0) ||
            !std::isfinite(uv.v0) ||
            !std::isfinite(uv.u1) ||
            !std::isfinite(uv.v1))
        {
            return false;
        }

        if (uv.u0 < 0.0f ||
            uv.v0 < 0.0f ||
            uv.u1 > 1.0f ||
            uv.v1 > 1.0f)
        {
            return false;
        }

        if (uv.u0 >= uv.u1 ||
            uv.v0 >= uv.v1)
        {
            return false;
        }

        return true;
    }

    bool IsValidDuration(
        float duration) noexcept
    {
        return
            std::isfinite(duration) &&
            duration > 0.0f;
    }
}

bool AnimationClip::AddFrame(
    const UVRect& uv,
    float duration)
{
    if (!IsValidUVRect(uv))
    {
        return false;
    }

    if (!IsValidDuration(
        duration))
    {
        return false;
    }

    AnimationFrame frame;

    frame.uv =
        uv;

    frame.duration =
        duration;

    m_frames.emplace_back(
        frame
    );

    return true;
}

void AnimationClip::ClearFrames()
{
    m_frames.clear();
}

const AnimationFrame*
AnimationClip::GetFrame(
    std::size_t index) const noexcept
{
    if (index >=
        m_frames.size())
    {
        return nullptr;
    }

    return
        &m_frames[index];
}

bool AnimationClip::IsValid() const noexcept
{
    if (!m_texture)
    {
        return false;
    }

    if (m_frames.empty())
    {
        return false;
    }

    for (const AnimationFrame& frame :
        m_frames)
    {
        if (!IsValidUVRect(
            frame.uv))
        {
            return false;
        }

        if (!IsValidDuration(
            frame.duration))
        {
            return false;
        }
    }

    return true;
}