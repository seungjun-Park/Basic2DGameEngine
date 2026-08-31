#pragma once

#include "Engine/Scene/Entity.h"

class PhysicsSystem;
class Player;

class Enemy :
    public Entity
{
public:
    explicit Enemy(
        PhysicsSystem& physics
    );

    void Initialize() override;

    void SetTarget(
        Player* player
    );

    void Update(
        float deltaTime
    ) override;

    void FixedUpdate(
        float fixedDeltaTime
    ) override;

private:
    PhysicsSystem&
        m_physics;

    Player* m_target = nullptr;

    DirectX::XMFLOAT2
        m_moveDirection
    {
        0.0f,
        0.0f
    };

    float m_speed = 100.0f;
};