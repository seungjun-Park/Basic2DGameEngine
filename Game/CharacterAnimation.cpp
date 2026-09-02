#include "CharacterAnimation.h"

#include "Engine/Animation/AnimationClip.h"

namespace
{
    std::size_t ToIndex(
        CharacterAnimationState state)
    {
        return
            static_cast<std::size_t>(
                state
                );
    }

    std::size_t ToIndex(
        FacingDirection direction)
    {
        return
            static_cast<std::size_t>(
                direction
                );
    }
}

void CharacterAnimationSet::SetClip(
    CharacterAnimationState state,
    FacingDirection direction,
    AnimationClip* clip) noexcept
{
    const std::size_t stateIndex =
        ToIndex(state);

    const std::size_t directionIndex =
        ToIndex(direction);

    if (stateIndex >= StateCount ||
        directionIndex >= DirectionCount)
    {
        return;
    }

    m_clips[stateIndex][directionIndex] =
        clip;
}

AnimationClip*
CharacterAnimationSet::GetClip(
    CharacterAnimationState state,
    FacingDirection direction) const noexcept
{
    const std::size_t stateIndex =
        ToIndex(state);

    const std::size_t directionIndex =
        ToIndex(direction);

    if (stateIndex >= StateCount ||
        directionIndex >= DirectionCount)
    {
        return nullptr;
    }

    return
        m_clips[stateIndex][directionIndex];
}

bool CharacterAnimationSet::IsValid() const noexcept
{
    for (const auto& state :
        m_clips)
    {
        for (AnimationClip* clip :
            state)
        {
            if (!clip ||
                !clip->IsValid())
            {
                return false;
            }
        }
    }

    return true;
}