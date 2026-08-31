#pragma once

#include <DirectXMath.h>

enum class PhysicsBodyType
{
    Static,
    Kinematic,
    Dynamic
};

struct PhysicsBodyDesc
{
    PhysicsBodyType type =
        PhysicsBodyType::Dynamic;

    DirectX::XMFLOAT2 size
    {
        32.0f,
        32.0f
    };

    DirectX::XMFLOAT2 offset
    {
        0.0f,
        0.0f
    };

    float density = 1.0f;
    float friction = 0.3f;
    float restitution = 0.0f;

    bool fixedRotation = true;

    bool isSensor = false;

    bool enableContactEvents = true;
};
