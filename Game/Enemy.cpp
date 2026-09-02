#include "Enemy.h"

#include "Player.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Physics/PhysicsTypes.h"

#include "CharacterAnimation.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Animation/Animator.h"

#include <cmath>

Enemy::Enemy(
    PhysicsSystem& physics)
    :
    m_physics(physics)
{
}

void Enemy::Initialize()
{
    PhysicsBodyDesc desc;

    desc.type =
        PhysicsBodyType::Dynamic;

    desc.size =
    {
        64.0f,
        64.0f
    };

    desc.offset =
    {
        (
            transform.scale.x -
            desc.size.x
        ) * 0.5f,

        (
            transform.scale.y -
            desc.size.y
        ) * 0.5f
    };

    desc.density = 1.0f;
    desc.friction = 0.0f;
    desc.restitution = 0.0f;

    desc.fixedRotation = true;

    physicsBody =
        std::make_unique<PhysicsBody>();

    if (!physicsBody->Create(
        m_physics,
        *this,
        desc,
        transform))
    {
        physicsBody.reset();
    }
}

void Enemy::SetTarget(
    Player* player)
{
    m_target = player;
}

void Enemy::Update(
    float deltaTime)
{
    m_moveDirection =
    {
        0.0f,
        0.0f
    };

    if (!m_target)
    {
        UpdateAnimation();

        return;
    }

    float dx =
        m_target->
        transform.position.x -
        transform.position.x;

    float dy =
        m_target->
        transform.position.y -
        transform.position.y;

    const float lengthSquared =
        dx * dx +
        dy * dy;

    if (lengthSquared <=
        0.001f)
    {
        UpdateAnimation();

        return;
    }

    const float length =
        std::sqrt(
            lengthSquared
        );

    m_moveDirection =
    {
        dx / length,
        dy / length
    };

    UpdateAnimation();
}

void Enemy::FixedUpdate(
    float fixedDeltaTime)
{
    if (!physicsBody)
    {
        return;
    }

    physicsBody->SetLinearVelocity(
        m_moveDirection.x *
        m_speed,

        m_moveDirection.y *
        m_speed
    );
}

bool Enemy::SetAnimations(
    const CharacterAnimationSet& animations)
{
    if (!animations.IsValid())
    {
        return false;
    }

    m_animations =
        animations;

    if (!animator)
    {
        animator =
            std::make_unique<Animator>(
                sprite
            );
    }

    m_animationState =
        CharacterAnimationState::Idle;

    m_facingDirection =
        FacingDirection::Down;

    AnimationClip* clip =
        m_animations.GetClip(
            m_animationState,
            m_facingDirection
        );

    if (!clip)
    {
        animator.reset();

        return false;
    }

    return
        animator->Play(
            *clip,
            true
        );
}

void Enemy::UpdateAnimation()
{
    if (!animator ||
        !m_animations.IsValid())
    {
        return;
    }

    constexpr float
        MovementEpsilon =
        0.0001f;

    const float absX =
        std::abs(
            m_moveDirection.x
        );

    const float absY =
        std::abs(
            m_moveDirection.y
        );

    const bool isMoving =
        absX > MovementEpsilon ||
        absY > MovementEpsilon;

    CharacterAnimationState
        newState =
        CharacterAnimationState::Idle;

    FacingDirection
        newDirection =
        m_facingDirection;

    if (isMoving)
    {
        newState =
            CharacterAnimationState::Walk;

        //
        // Dominant-axis facing.
        //
        // Diagonal 이동 시 더 큰 축을
        // character facing으로 사용한다.
        //
        if (absX > absY)
        {
            if (m_moveDirection.x < 0.0f)
            {
                newDirection =
                    FacingDirection::Left;
            }
            else
            {
                newDirection =
                    FacingDirection::Right;
            }
        }
        else
        {
            if (m_moveDirection.y < 0.0f)
            {
                newDirection =
                    FacingDirection::Up;
            }
            else
            {
                newDirection =
                    FacingDirection::Down;
            }
        }
    }

    m_animationState =
        newState;

    m_facingDirection =
        newDirection;

    AnimationClip* clip =
        m_animations.GetClip(
            m_animationState,
            m_facingDirection
        );

    if (!clip)
    {
        return;
    }

    animator->Play(
        *clip
    );
}