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

    //
    // Collider의 기본 top-left 위치에서
    // 추가로 적용되는 pixel offset.
    //
    // offset { 0, 0 }이면
    // collider의 좌상단과 Transform.position이 일치한다.
    //

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
