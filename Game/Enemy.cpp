#include "Enemy.h"

#include "Player.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Physics/PhysicsTypes.h"

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
        return;
    }

    float dx =
        m_target->transform.position.x -
        transform.position.x;

    float dy =
        m_target->transform.position.y -
        transform.position.y;

    const float lengthSquared =
        dx * dx +
        dy * dy;

    if (lengthSquared <=
        0.001f)
    {
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