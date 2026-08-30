#pragma once

#include "Engine/Scene/Entity.h"

class Player :
    public Entity
{
public:
    Player() = default;
    ~Player() override = default;

    void Initialize() override;

    void Update(
        float deltaTime
    ) override;

    bool IsAttacking() const;

    float GetAttackRange() const;

private:
    float m_speed = 200.0f;

    bool m_isAttacking = false;

    float m_attackRange = 100.0f;
};