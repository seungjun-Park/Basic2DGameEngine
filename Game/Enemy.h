#pragma once

#include "Engine/Scene/Entity.h"

class Player;

class Enemy :
    public Entity
{
public:
    Enemy() = default;

    void SetTarget(
        Player* player
    );

    void Update(
        float deltaTime
    ) override;

private:
    Player* m_target = nullptr;

    float m_speed = 100.0f;
};