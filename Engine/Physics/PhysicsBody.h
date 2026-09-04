#pragma once

#include "Engine/Scene/EntityHandle.h"

#include <box2d/box2d.h>


struct Transform;
class Entity;

struct PhysicsBodyDesc;
class PhysicsSystem;

struct PhysicsBodyUserData
{
    EntityHandle entityHandle{};
};

class PhysicsBody
{
public:
    PhysicsBody();
    ~PhysicsBody();

    PhysicsBody(
        const PhysicsBody&
    ) = delete;

    PhysicsBody&
        operator=(
            const PhysicsBody&
            ) = delete;

    bool Create(
        PhysicsSystem& physics,
        Entity& owner,
        const PhysicsBodyDesc& desc,
        const Transform& transform
    );

    void Destroy();

    void SyncTransform(
        Transform& transform
    ) const;


    void SetLinearVelocity(
        float xPixelsPerSecond,
        float yPixelsPerSecond
    );
    void SetPosition(
        float xPixels,
        float yPixels
    );
    void SetRotation(
        float radians
    );

    bool IsValid() const;

    b2BodyId GetBodyId() const
    {
        return m_bodyId;
    }

    b2ShapeId GetShapeId() const
    {
        return m_shapeId;
    }

private:
    PhysicsBodyUserData m_userData{};

    b2BodyId m_bodyId =
        b2_nullBodyId;

    b2ShapeId m_shapeId =
        b2_nullShapeId;
};