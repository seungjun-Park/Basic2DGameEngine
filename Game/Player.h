#pragma once

#include "Engine/Scene/Entity.h"

class PhysicsSystem;

class Player :
    public Entity
{
public:
    explicit Player(
        PhysicsSystem& physics
    );
    ~Player() override = default;

    void Initialize() override;

    void Update(
        float deltaTime
    ) override;

    void FixedUpdate(
        float fixedDeltaTime
    ) override;

    bool IsAttacking() const;

    float GetAttackRange() const;

private:
    PhysicsSystem& m_physics;

    DirectX::XMFLOAT2
        m_moveDirection
    {
        0.0f,
        0.0f
    };

    float m_speed =
        220.0f;

    bool m_isAttacking = false;

    float m_attackRange = 100.0f;
};