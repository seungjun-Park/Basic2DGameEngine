#include "Entity.h"

#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Graphics/Texture.h"

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