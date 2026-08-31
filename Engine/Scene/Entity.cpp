#include "Entity.h"

#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Physics/PhysicsBody.h"

Entity::~Entity() = default;

void Entity::Render(
    SpriteRenderer& renderer)
{
    if (!active)
        return;

    if (!sprite.visible)
        return;

    if (!sprite.texture)
        return;

    renderer.Draw(
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