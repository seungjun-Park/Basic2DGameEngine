#pragma once

#include <box2d/box2d.h>

class Entity;
class Scene;
class EventBus;

class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    PhysicsSystem(
        const PhysicsSystem&
    ) = delete;

    PhysicsSystem&
        operator=(
            const PhysicsSystem&
            ) = delete;

    bool Initialize();

    void Shutdown();

    void Step(
        float fixedDeltaTime
    );

    void DispatchContactEvents(
        Scene& scene,
        EventBus& eventBus
    );

    b2WorldId GetWorldId() const
    {
        return m_worldId;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

private:
    void HandleBeginContact(
        Scene& scene,
        EventBus& eventBus,
        b2ShapeId shapeA,
        b2ShapeId shapeB
    );

    void HandleEndContact(
        Scene& scene,
        EventBus& eventBus,
        b2ShapeId shapeA,
        b2ShapeId shapeB
    );

    Entity* ResolveBodyEntity(
        Scene& scene,
        b2BodyId body
    ) const;

private:
    b2WorldId m_worldId =
        b2_nullWorldId;

    bool m_initialized = false;

    int m_subStepCount = 4;
};
