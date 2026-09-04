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

void Camera::Resize(
    float screenWidth,
    float screenHeight)
{
    if (screenWidth <= 0.0f ||
        screenHeight <= 0.0f)
    {
        return;
    }

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

float Camera::GetLeft() const
{
    return
        m_position.x -
        m_screenWidth * 0.5f;
}

float Camera::GetRight() const
{
    return
        m_position.x +
        m_screenWidth * 0.5f;
}

float Camera::GetTop() const
{
    return
        m_position.y -
        m_screenHeight * 0.5f;
}

float Camera::GetBottom() const
{
    return
        m_position.y +
        m_screenHeight * 0.5f;
}