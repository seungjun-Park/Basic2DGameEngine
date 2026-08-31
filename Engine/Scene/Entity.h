#pragma once

#include "Engine/Components/Transform.h"
#include "Engine/Components/Sprite.h"

#include <memory>

class SpriteRenderer;
class PhysicsBody;
class RenderQueue;

class Entity
{
public:
    Entity() = default;

    virtual ~Entity();

    virtual void Initialize()
    {
    }

    virtual void FixedUpdate(
        float fixedDeltaTime)
    {
    }

    virtual void Update(
        float deltaTime)
    {
    }

    virtual void LateUpdate(
        float deltaTime)
    {
    }

    virtual void SubmitRender(
        RenderQueue& renderQueue
    );

    void Destroy();

    bool IsDestroyed() const;

    virtual void OnCollisionEnter(
        Entity& other)
    {
    }

    virtual void OnCollisionExit(
        Entity& other)
    {
    }

    void SyncPhysicsTransform();

public:
    Transform transform;
    Sprite sprite;

    bool active = true;

    std::unique_ptr<PhysicsBody>
        physicsBody;

private:
    bool m_destroyed = false;
};