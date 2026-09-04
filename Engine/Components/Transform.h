#pragma once

#include <DirectXMath.h>

struct Transform
{
    Transform() = default;

    Transform(
        float x,
        float y,
        float width,
        float height,
        float rotation = 0.0f
    )
        : position(x, y),
        scale(width, height),
        rotation(rotation)
    {
    }

    DirectX::XMFLOAT2 position{ 0.0f, 0.0f };
    DirectX::XMFLOAT2 scale{ 1.0f, 1.0f };

    float rotation = 0.0f;
};