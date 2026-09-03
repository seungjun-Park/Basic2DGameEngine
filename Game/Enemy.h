#pragma once

#include "CharacterAnimation.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/EntityHandle.h"

class PhysicsSystem;
class Scene;
class Player;

class Enemy :
    public Entity
{
public:
    explicit Enemy(
        Scene& scene,
        PhysicsSystem& physics
    );

    void Initialize() override;

    bool SetTarget(
        EntityHandle target
    );

    void Update(
        float deltaTime
    ) override;

    void FixedUpdate(
        float fixedDeltaTime
    ) override;

    bool SetAnimations(
        const CharacterAnimationSet& animations
    );

private:
    void UpdateAnimation();

    Player* ResolveTarget();

private:
    Scene&
        m_scene;

    PhysicsSystem&
        m_physics;

    EntityHandle
        m_target{};

    DirectX::XMFLOAT2
        m_moveDirection
    {
        0.0f,
        0.0f
    };

    float m_speed = 100.0f;

    CharacterAnimationSet
        m_animations;

    CharacterAnimationState
        m_animationState =
        CharacterAnimationState::Idle;

    FacingDirection
        m_facingDirection =
        FacingDirection::Down;
};