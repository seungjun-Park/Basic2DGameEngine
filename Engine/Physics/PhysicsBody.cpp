#include "PhysicsBody.h"

#include "PhysicsSystem.h"
#include "PhysicsUnits.h"

#include "Engine/Components/Transform.h"
#include "Engine/Scene/Entity.h"

PhysicsBody::PhysicsBody() = default;

PhysicsBody::~PhysicsBody()
{
    Destroy();
}

namespace
{
    b2BodyType ConvertBodyType(
        PhysicsBodyType type)
    {
        switch (type)
        {
        case PhysicsBodyType::Static:
            return b2_staticBody;

        case PhysicsBodyType::Kinematic:
            return b2_kinematicBody;

        case PhysicsBodyType::Dynamic:
        default:
            return b2_dynamicBody;
        }
    }
}

bool PhysicsBody::Create(
    PhysicsSystem& physics,
    Entity& owner,
    const PhysicsBodyDesc& desc,
    const Transform& transform)
{
    Destroy();

    m_physics =
        &physics;

    m_owner =
        &owner;

    b2BodyDef bodyDef =
        b2DefaultBodyDef();

    bodyDef.type =
        ConvertBodyType(
            desc.type
        );

    bodyDef.position =
    {
        PhysicsUnits::ToMeters(
            transform.position.x
        ),

        PhysicsUnits::ToMeters(
            transform.position.y
        )
    };

    bodyDef.rotation =
        b2MakeRot(
            transform.rotation
        );

    bodyDef.fixedRotation =
        desc.fixedRotation;

    // 충돌 이벤트에서 Entity를 찾기 위해
    // owner 포인터를 Box2D userData로 연결
    bodyDef.userData =
        &owner;

    m_bodyId =
        b2CreateBody(
            physics.GetWorldId(),
            &bodyDef
        );

    if (B2_IS_NULL(
        m_bodyId))
    {
        m_physics = nullptr;
        m_owner = nullptr;

        return false;
    }

    b2ShapeDef shapeDef =
        b2DefaultShapeDef();

    shapeDef.density =
        desc.density;

    shapeDef.material.friction =
        desc.friction;

    shapeDef.material.restitution =
        desc.restitution;

    shapeDef.isSensor =
        desc.isSensor;

    shapeDef.enableContactEvents =
        desc.enableContactEvents;

    const float halfWidth =
        PhysicsUnits::ToMeters(
            desc.size.x * 0.5f
        );

    const float halfHeight =
        PhysicsUnits::ToMeters(
            desc.size.y * 0.5f
        );

    const b2Vec2 center
    {
        PhysicsUnits::ToMeters(
            desc.size.x * 0.5f +
            desc.offset.x
        ),

        PhysicsUnits::ToMeters(
            desc.size.y * 0.5f +
            desc.offset.y
        )
    };

    const b2Polygon box =
        b2MakeOffsetBox(
            halfWidth,
            halfHeight,
            center,
            b2MakeRot(0.0f)
        );

    m_shapeId =
        b2CreatePolygonShape(
            m_bodyId,
            &shapeDef,
            &box
        );

    if (B2_IS_NULL(
        m_shapeId))
    {
        b2DestroyBody(
            m_bodyId
        );

        m_bodyId =
            b2_nullBodyId;

        m_physics = nullptr;
        m_owner = nullptr;

        return false;
    }

    return true;
}

void PhysicsBody::Destroy()
{
    if (B2_IS_NON_NULL(
        m_bodyId))
    {
        if (b2Body_IsValid(
            m_bodyId))
        {
            // Box2D가 더 이상 삭제될 Entity를
            // 가리키지 않도록 먼저 끊는다.
            b2Body_SetUserData(
                m_bodyId,
                nullptr
            );

            b2DestroyBody(
                m_bodyId
            );
        }

        m_bodyId =
            b2_nullBodyId;

        m_shapeId =
            b2_nullShapeId;
    }

    m_owner = nullptr;
    m_physics = nullptr;
}

bool PhysicsBody::IsValid() const
{
    return
        B2_IS_NON_NULL(
            m_bodyId
        ) &&
        b2Body_IsValid(
            m_bodyId
        );
}

void PhysicsBody::SetLinearVelocity(
    float xPixelsPerSecond,
    float yPixelsPerSecond)
{
    if (!IsValid())
    {
        return;
    }

    b2Vec2 velocity
    {
        PhysicsUnits::ToMeters(
            xPixelsPerSecond
        ),

        PhysicsUnits::ToMeters(
            yPixelsPerSecond
        )
    };

    b2Body_SetLinearVelocity(
        m_bodyId,
        velocity
    );
}

void PhysicsBody::SetPosition(
    float xPixels,
    float yPixels)
{
    if (!IsValid())
    {
        return;
    }

    const b2Vec2 position
    {
        PhysicsUnits::ToMeters(
            xPixels
        ),

        PhysicsUnits::ToMeters(
            yPixels
        )
    };

    const b2Rot rotation =
        b2Body_GetRotation(
            m_bodyId
        );

    b2Body_SetTransform(
        m_bodyId,
        position,
        rotation
    );
}

void PhysicsBody::SetRotation(
    float radians)
{
    if (!IsValid())
    {
        return;
    }

    const b2Vec2 position =
        b2Body_GetPosition(
            m_bodyId
        );

    b2Body_SetTransform(
        m_bodyId,
        position,
        b2MakeRot(radians)
    );
}

void PhysicsBody::SyncTransform(
    Transform& transform) const
{
    if (!IsValid())
    {
        return;
    }

    const b2Vec2 position =
        b2Body_GetPosition(
            m_bodyId
        );

    transform.position =
    {
        PhysicsUnits::ToPixels(
            position.x
        ),

        PhysicsUnits::ToPixels(
            position.y
        )
    };

    const b2Rot rotation =
        b2Body_GetRotation(
            m_bodyId
        );

    transform.rotation =
        b2Rot_GetAngle(
            rotation
        );
}