#include "Player.h"

#include "Engine/Platform/Windows/WinInput.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Physics/PhysicsSystem.h"
#include "Engine/Physics/PhysicsTypes.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <cmath>

Player::Player(
    PhysicsSystem& physics)
    : m_physics(
        physics
    )
{
}

void Player::Initialize()
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

    desc.enableContactEvents = true;

    desc.offset =
    {
        16.0f,
        16.0f
    };

    physicsBody =
        std::make_unique<PhysicsBody>();


    if (!physicsBody->Create(
        m_physics,
        *this,
        desc,
        transform))
    {
        physicsBody.reset();

        OutputDebugStringA(
            "[Player] Failed to create PhysicsBody.\n"
        );
    }
}

void Player::Update(
    float deltaTime)
{
    m_moveDirection =
    {
        0.0f,
        0.0f
    };

    if (WinInput::IsKeyDown('W'))
    {
        m_moveDirection.y -= 1.0f;
    }

    if (WinInput::IsKeyDown('S'))
    {
        m_moveDirection.y += 1.0f;
    }

    if (WinInput::IsKeyDown('A'))
    {
        m_moveDirection.x -= 1.0f;
    }

    if (WinInput::IsKeyDown('D'))
    {
        m_moveDirection.x += 1.0f;
    }

    const float lengthSquared =
        m_moveDirection.x *
        m_moveDirection.x
        +
        m_moveDirection.y *
        m_moveDirection.y;

    if (lengthSquared > 0.0f)
    {
        const float length =
            std::sqrt(
                lengthSquared
            );

        m_moveDirection.x /=
            length;

        m_moveDirection.y /=
            length;
    }

    // Press 기반 공격 같은 것은
    // 계속 Update에 둔다.
    m_isAttacking =
        WinInput::IsKeyPressed(
            VK_SPACE
        );
}

void Player::FixedUpdate(
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

bool Player::IsAttacking() const
{
    return m_isAttacking;
}

float Player::GetAttackRange() const
{
    return m_attackRange;
}

void Player::OnCollisionEnter(
    Entity& other)
{
    OutputDebugStringA(
        "[Player] Collision Enter\n"
    );
}

void Player::OnCollisionExit(
    Entity& other)
{
    OutputDebugStringA(
        "[Player] Collision Exit\n"
    );
}