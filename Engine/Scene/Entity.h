#pragma once

#include "Engine/Components/Transform.h"
#include "Engine/Components/Sprite.h"
#include "Engine/Animation/Animator.h"
#include "Engine/Scene/EntityHandle.h"

#include <memory>

class SpriteRenderer;
class PhysicsBody;
class RenderQueue;
class Animator;
class Scene;

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

    EntityHandle GetHandle() const noexcept
    {
        return m_handle;
    }

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

    std::unique_ptr<Animator>
        animator;

private:

    friend class Scene;

    void SetHandle(
        EntityHandle handle
    ) noexcept
    {
        m_handle = handle;
    }

private:
    EntityHandle m_handle{};

    bool m_destroyed = false;
};