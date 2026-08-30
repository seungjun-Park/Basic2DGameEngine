#include "Scene.h"

#include "Entity.h"
#include "Engine/Renderer/SpriteRenderer.h"

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