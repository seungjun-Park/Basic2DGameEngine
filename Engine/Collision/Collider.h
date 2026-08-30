#pragma once

#include <DirectXMath.h>

struct Collider
{
    DirectX::XMFLOAT2 offset
    {
        0.0f,
        0.0f
    };

    DirectX::XMFLOAT2 size
    {
        0.0f,
        0.0f
    };

    bool enabled = true;
};
