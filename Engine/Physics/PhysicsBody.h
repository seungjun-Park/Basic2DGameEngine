#pragma once

#include "PhysicsTypes.h"

#include <box2d/box2d.h>

class PhysicsSystem;
struct Transform;
class Entity;

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

    bool IsValid() const;

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

    void SyncTransform(
        Transform& transform
    ) const;

    b2BodyId GetBodyId() const
    {
        return m_bodyId;
    }

    b2ShapeId GetShapeId() const
    {
        return m_shapeId;
    }

private:
    PhysicsSystem* m_physics =
        nullptr;

    Entity* m_owner =
        nullptr;

    b2BodyId m_bodyId =
        b2_nullBodyId;

    b2ShapeId m_shapeId =
        b2_nullShapeId;
};