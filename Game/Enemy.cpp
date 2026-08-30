#include "Enemy.h"

#include "Player.h"

#include <cmath>

void Enemy::SetTarget(
    Player* player)
{
    m_target = player;
}

void Enemy::Update(
    float deltaTime)
{
    if (!m_target)
        return;

    float dx =
        m_target->transform.position.x -
        transform.position.x;

    float dy =
        m_target->transform.position.y -
        transform.position.y;

    float length =
        std::sqrt(
            dx * dx +
            dy * dy
        );

    if (length <= 0.001f)
        return;

    dx /= length;
    dy /= length;

    transform.position.x +=
        dx *
        m_speed *
        deltaTime;

    transform.position.y +=
        dy *
        m_speed *
        deltaTime;
}