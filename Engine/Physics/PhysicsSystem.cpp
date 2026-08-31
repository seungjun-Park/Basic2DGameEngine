#include "PhysicsSystem.h"

#include "Engine/Scene/Entity.h"

#include <Windows.h>

PhysicsSystem::PhysicsSystem() = default;

PhysicsSystem::~PhysicsSystem()
{
    Shutdown();
}

bool PhysicsSystem::Initialize()
{
    if (m_initialized)
    {
        return true;
    }

    b2WorldDef worldDef =
        b2DefaultWorldDef();

    // Top-down 2D 게임이므로 중력 없음
    worldDef.gravity =
    {
        0.0f,
        0.0f
    };

    m_worldId =
        b2CreateWorld(
            &worldDef
        );

    if (B2_IS_NULL(
        m_worldId))
    {
        OutputDebugStringA(
            "[Physics] Failed to create Box2D world.\n"
        );

        return false;
    }

    m_initialized = true;

    OutputDebugStringA(
        "[Physics] Box2D world initialized.\n"
    );

    return true;
}

void PhysicsSystem::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    if (B2_IS_NON_NULL(
        m_worldId))
    {
        b2DestroyWorld(
            m_worldId
        );

        m_worldId =
            b2_nullWorldId;
    }

    m_initialized = false;
}

void PhysicsSystem::Step(
    float fixedDeltaTime)
{
    if (!m_initialized)
    {
        return;
    }

    if (fixedDeltaTime <= 0.0f)
    {
        return;
    }

    b2World_Step(
        m_worldId,
        fixedDeltaTime,
        m_subStepCount
    );
}

void PhysicsSystem::DispatchContactEvents()
{
    if (!m_initialized)
    {
        return;
    }

    const b2ContactEvents events =
        b2World_GetContactEvents(
            m_worldId
        );

    for (
        int i = 0;
        i < events.beginCount;
        ++i)
    {
        const b2ContactBeginTouchEvent&
            event =
            events.beginEvents[i];

        if (!b2Shape_IsValid(
            event.shapeIdA) ||
            !b2Shape_IsValid(
                event.shapeIdB))
        {
            continue;
        }

        HandleBeginContact(
            event.shapeIdA,
            event.shapeIdB
        );
    }

    for (
        int i = 0;
        i < events.endCount;
        ++i)
    {
        const b2ContactEndTouchEvent&
            event =
            events.endEvents[i];

        if (!b2Shape_IsValid(
            event.shapeIdA) ||
            !b2Shape_IsValid(
                event.shapeIdB))
        {
            continue;
        }

        HandleEndContact(
            event.shapeIdA,
            event.shapeIdB
        );
    }
}

void PhysicsSystem::HandleBeginContact(
    b2ShapeId shapeA,
    b2ShapeId shapeB)
{
    if (!b2Shape_IsValid(shapeA) ||
        !b2Shape_IsValid(shapeB))
    {
        return;
    }

    const b2BodyId bodyA =
        b2Shape_GetBody(
            shapeA
        );

    const b2BodyId bodyB =
        b2Shape_GetBody(
            shapeB
        );

    Entity* entityA =
        static_cast<Entity*>(
            b2Body_GetUserData(
                bodyA
            )
            );

    Entity* entityB =
        static_cast<Entity*>(
            b2Body_GetUserData(
                bodyB
            )
            );

    if (!entityA ||
        !entityB)
    {
        return;
    }

    entityA->OnCollisionEnter(
        *entityB
    );

    entityB->OnCollisionEnter(
        *entityA
    );
}

void PhysicsSystem::HandleEndContact(
    b2ShapeId shapeA,
    b2ShapeId shapeB)
{
    // Box2D에서는 body/shape가 파괴될 때도
    // EndContact가 생성될 수 있다.
    //
    // 따라서 End event의 Shape ID가 이미
    // 무효화되었을 가능성이 있다.
    if (!b2Shape_IsValid(shapeA) ||
        !b2Shape_IsValid(shapeB))
    {
        return;
    }

    const b2BodyId bodyA =
        b2Shape_GetBody(
            shapeA
        );

    const b2BodyId bodyB =
        b2Shape_GetBody(
            shapeB
        );

    Entity* entityA =
        static_cast<Entity*>(
            b2Body_GetUserData(
                bodyA
            )
            );

    Entity* entityB =
        static_cast<Entity*>(
            b2Body_GetUserData(
                bodyB
            )
            );

    if (!entityA ||
        !entityB)
    {
        return;
    }

    entityA->OnCollisionExit(
        *entityB
    );

    entityB->OnCollisionExit(
        *entityA
    );
}