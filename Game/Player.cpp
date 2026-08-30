#include "Player.h"

#include "Engine/Platform/Windows/WinInput.h"

#include <Windows.h>
#include <DirectXMath.h>
#include <cmath>

void Player::Initialize()
{
}

void Player::Update(
    float deltaTime)
{
    DirectX::XMFLOAT2 direction
    {
        0.0f,
        0.0f
    };

    if (WinInput::IsKeyDown('W'))
    {
        direction.y -= 1.0f;
    }

    if (WinInput::IsKeyDown('S'))
    {
        direction.y += 1.0f;
    }

    if (WinInput::IsKeyDown('A'))
    {
        direction.x -= 1.0f;
    }

    if (WinInput::IsKeyDown('D'))
    {
        direction.x += 1.0f;
    }

    float length =
        std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y
        );

    if (length > 0.0f)
    {
        direction.x /= length;
        direction.y /= length;
    }

    transform.position.x +=
        direction.x *
        m_speed *
        deltaTime;

    transform.position.y +=
        direction.y *
        m_speed *
        deltaTime;

    m_isAttacking =
        WinInput::IsKeyPressed(VK_SPACE);
}

bool Player::IsAttacking() const
{
    return m_isAttacking;
}

float Player::GetAttackRange() const
{
    return m_attackRange;
}