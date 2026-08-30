#include "Scene.h"

#include "Entity.h"
#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Debug/DebugRenderer.h"

#include <algorithm>

void Scene::Render(
    SpriteRenderer& renderer)
{
    for (auto& entity : m_entities)
    {
        if (!entity->active)
            continue;

        entity->Render(
            renderer
        );
    }
}

void Scene::Update(
    float deltaTime)
{
    for (auto& entity : m_entities)
    {
        if (!entity->active)
            continue;

        entity->Update(
            deltaTime
        );
    }

    std::erase_if(
        m_entities,
        [](const std::unique_ptr<Entity>& entity)
        {
            return entity->IsDestroyed();
        }
    );
}

void Scene::DebugRender(
    SpriteRenderer& renderer,
    DebugRenderer& debugRenderer)
{
    for (auto& entity : m_entities)
    {
        if (!entity->active)
            continue;

        if (!entity->collider.enabled)
            continue;

        DirectX::XMFLOAT2 center
        {
            entity->transform.position.x +
                entity->collider.offset.x,

            entity->transform.position.y +
                entity->collider.offset.y
        };

        debugRenderer.DrawRect(
            renderer,
            center,
            entity->collider.size
        );
    }
}