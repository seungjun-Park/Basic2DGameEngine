#pragma once

#include "Engine/Animation/Animator.h"
#include "Engine/Components/Sprite.h"
#include "Engine/Components/Transform.h"
#include "Engine/Scene/EntityHandle.h"

#include <memory>
#include <string>

class PhysicsBody;
class RenderQueue;
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
        float fixedDeltaTime
    )
    {
    }

    virtual void Update(
        float deltaTime
    )
    {
    }

    virtual void LateUpdate(
        float deltaTime
    )
    {
    }

    virtual void SubmitRender(
        RenderQueue& renderQueue
    );

    void SyncPhysicsTransform();

    void Destroy();

    void SetAssignedAnimationClipPath(
        const std::wstring& path
    );

    void ClearAssignedAnimationClipPath()
        noexcept;

    [[nodiscard]]
    bool IsDestroyed() const;

    [[nodiscard]]
    EntityHandle GetHandle()
        const noexcept
    {
        return m_handle;
    }

    [[nodiscard]]
    const std::wstring&
        GetAssignedAnimationClipPath()
        const noexcept;

    virtual void OnCollisionEnter(
        Entity& other
    )
    {
    }

    virtual void OnCollisionExit(
        Entity& other
    )
    {
    }

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
        m_handle =
            handle;
    }

private:
    EntityHandle
        m_handle{};

    std::wstring
        m_assignedAnimationClipPath;

    bool m_destroyed =
        false;
};