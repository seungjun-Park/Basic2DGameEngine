#include "Animator.h"

#include "AnimationClip.h"
#include "Engine/Components/Sprite.h"

#include <cmath>

Animator::Animator(
    Sprite& sprite) noexcept
    :
    m_sprite(
        &sprite
    )
{
}

void Animator::Update(
    float deltaTime)
{
    if (!m_playing ||
        !m_clip ||
        !m_sprite)
    {
        return;
    }

    if (!std::isfinite(
        deltaTime) ||
        deltaTime <= 0.0f)
    {
        return;
    }

    const std::size_t
        frameCount =
        m_clip->
        GetFrameCount();

    if (frameCount == 0)
    {
        m_playing = false;

        return;
    }

    const std::uint64_t
        currentRevision =
        m_clip->GetRevision();

    if (m_clipRevision !=
        currentRevision)
    {
        m_clipRevision =
            currentRevision;

        if (m_frameIndex >=
            frameCount)
        {
            if (m_clip->IsLooping())
            {
                m_frameIndex %=
                    frameCount;
            }
            else
            {
                m_frameIndex =
                    frameCount - 1;
            }
        }

        ApplyCurrentFrame();
    }

    m_frameElapsed +=
        deltaTime;

    while (m_playing)
    {
        const AnimationFrame* frame =
            m_clip->GetFrame(
                m_frameIndex
            );

        if (!frame)
        {
            m_playing = false;

            return;
        }

        if (m_frameElapsed <
            frame->duration)
        {
            break;
        }

        m_frameElapsed -=
            frame->duration;

        if (m_frameIndex + 1 <
            frameCount)
        {
            ++m_frameIndex;

            ApplyCurrentFrame();

            continue;
        }

        if (m_clip->IsLooping())
        {
            m_frameIndex =
                0;

            ApplyCurrentFrame();

            continue;
        }

        m_frameIndex = frameCount - 1;

        m_frameElapsed = 0.0f;

        m_playing = false;

        ApplyCurrentFrame();

        break;
    }
}

bool Animator::Play(
    const AnimationClip& clip,
    bool restart)
{
    if (!clip.IsValid())
    {
        return false;
    }

    if (m_clip == &clip &&
        !restart)
    {
        return true;
    }

    m_clip =
        &clip;

    m_clipRevision =
        clip.GetRevision();

    m_frameIndex =
        0;

    m_frameElapsed =
        0.0f;

    m_playing =
        true;

    ApplyCurrentFrame();

    return true;
}

void Animator::Stop() noexcept
{
    m_playing = false;
}

void Animator::ApplyCurrentFrame()
{
    if (!m_sprite ||
        !m_clip)
    {
        return;
    }

    const AnimationFrame* frame =
        m_clip->GetFrame(
            m_frameIndex
        );

    if (!frame)
    {
        return;
    }

    m_sprite->texture =
        m_clip->GetTexture();

    m_sprite->uv =
        frame->uv;
}