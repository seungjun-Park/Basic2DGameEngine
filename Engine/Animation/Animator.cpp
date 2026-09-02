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

bool Animator::Play(
    const AnimationClip& clip,
    bool restart)
{
    if (!clip.IsValid())
    {
        return false;
    }

    //
    // 이미 같은 animation이 정상 재생 중이고
    // restart 요청이 없다면 아무것도 하지 않는다.
    //
    if (m_clip == &clip &&
        m_playing &&
        !restart)
    {
        return true;
    }

    m_clip =
        &clip;

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
    //
    // 현재 보이는 frame은 유지한다.
    //
    m_playing =
        false;
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

    m_frameElapsed +=
        deltaTime;

    //
    // deltaTime이 한 frame duration보다
    // 큰 경우에도 animation time을 잃지 않는다.
    //
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

        //
        // Non-loop clip은 마지막 frame을
        // 화면에 남기고 종료한다.
        //
        m_frameIndex =
            frameCount - 1;

        m_frameElapsed =
            0.0f;

        m_playing =
            false;

        ApplyCurrentFrame();

        break;
    }
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