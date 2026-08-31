#include "Entity.h"

#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Renderer/RenderQueue.h"

Entity::~Entity() = default;

void Entity::SubmitRender(
    RenderQueue& renderQueue)
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

void Entity::Destroy()
{
    m_destroyed = true;
}

bool Entity::IsDestroyed() const
{
    return m_destroyed;
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