#pragma once

#include <array>
#include <cstddef>

class AnimationClip;

enum class CharacterAnimationState
{
    Idle = 0,
    Walk,

    Count
};

enum class FacingDirection
{
    Down = 0,
    Left,
    Right,
    Up,

    Count
};

class CharacterAnimationSet
{
public:
    void SetClip(
        CharacterAnimationState state,
        FacingDirection direction,
        AnimationClip* clip
    ) noexcept;

    AnimationClip* GetClip(
        CharacterAnimationState state,
        FacingDirection direction
    ) const noexcept;

    bool IsValid() const noexcept;

private:
    static constexpr std::size_t
        StateCount =
        static_cast<std::size_t>(
            CharacterAnimationState::Count
            );

    static constexpr std::size_t
        DirectionCount =
        static_cast<std::size_t>(
            FacingDirection::Count
            );

    std::array<
        std::array<
        AnimationClip*,
        DirectionCount
        >,
        StateCount
    > m_clips{};
};