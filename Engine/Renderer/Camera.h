#pragma once

#include <DirectXMath.h>

class Camera
{
public:
    Camera() = default;

    void Initialize(
        float screenWidth,
        float screenHeight
    );

    void SetPosition(
        float x,
        float y
    );

    const DirectX::XMFLOAT2&
        GetPosition() const;

    DirectX::XMMATRIX
        GetViewMatrix() const;

    DirectX::XMMATRIX
        GetProjectionMatrix() const;

    float GetLeft() const;
    float GetRight() const;

    float GetTop() const;
    float GetBottom() const;

    void Resize(
        float screenWidth,
        float screenHeight
    );

private:
    DirectX::XMFLOAT2 m_position
    {
        0.0f,
        0.0f
    };

    float m_screenWidth = 0.0f;
    float m_screenHeight = 0.0f;
};