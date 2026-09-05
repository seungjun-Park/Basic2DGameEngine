#include "Entity.h"

#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Renderer/RenderQueue.h"

Entity::~Entity() = default;

void Entity::SubmitRender(
    RenderQueue& renderQueue
)
{
    if (!active)
    {
        return;
    }

    renderQueue.Submit(
        sprite,
        transform
    );
}

void Entity::SyncPhysicsTransform()
{
    if (!physicsBody)
    {
        return;
    }

    if (!physicsBody->IsValid())
    {
        return;
    }

    physicsBody->SyncTransform(
        transform
    );
}

void Entity::Destroy()
{
    m_destroyed =
        true;
}

void Entity::SetAssignedAnimationClipPath(
    const std::wstring& path
)
{
    m_assignedAnimationClipPath =
        path;
}

void Entity::
ClearAssignedAnimationClipPath()
noexcept
{
    m_assignedAnimationClipPath.clear();
}

bool Entity::IsDestroyed()
const
{
    return
        m_destroyed;
}

const std::wstring&
Entity::GetAssignedAnimationClipPath()
const noexcept
{
    return
        m_assignedAnimationClipPath;
}