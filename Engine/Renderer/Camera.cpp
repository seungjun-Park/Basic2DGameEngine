#include "Camera.h"

using namespace DirectX;

void Camera::Initialize(
    float screenWidth,
    float screenHeight)
{
    m_screenWidth =
        screenWidth;

    m_screenHeight =
        screenHeight;
}

void Camera::SetPosition(
    float x,
    float y)
{
    m_position =
    {
        x,
        y
    };
}

const XMFLOAT2&
Camera::GetPosition() const
{
    return m_position;
}

XMMATRIX
Camera::GetViewMatrix() const
{
    return XMMatrixTranslation(
        -m_position.x,
        -m_position.y,
        0.0f
    );
}

XMMATRIX
Camera::GetProjectionMatrix() const
{
    float halfWidth =
        m_screenWidth * 0.5f;

    float halfHeight =
        m_screenHeight * 0.5f;

    return
        XMMatrixOrthographicOffCenterLH(
            -halfWidth,
            halfWidth,
            halfHeight,
            -halfHeight,
            0.0f,
            1.0f
        );
}