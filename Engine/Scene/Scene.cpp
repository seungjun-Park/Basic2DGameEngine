#include "Scene.h"

#include "Entity.h"
#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Debug/DebugRenderer.h"

#include <algorithm>

void Scene::SubmitRender(
    RenderQueue& renderQueue)
{
    for (auto& entity :
        m_entities)
    {
        if (!entity->active)
        {
            continue;
        }

        entity->SubmitRender(
            renderQueue
        );
    }
}

void Scene::FixedUpdate(
    float fixedDeltaTime)
{
    for (auto& entity :
        m_entities)
    {
        if (!entity->active)
        {
            continue;
        }

        entity->FixedUpdate(
            fixedDeltaTime
        );
    }
}

void Scene::Update(
    float deltaTime)
{
    for (auto& entity :
        m_entities)
    {
        if (!entity->active)
        {
            continue;
        }

        entity->Update(
            deltaTime
        );
    }
}

void Scene::LateUpdate(
    float deltaTime)
{
    for (auto& entity :
        m_entities)
    {
        if (!entity->active)
        {
            continue;
        }

        entity->LateUpdate(
            deltaTime
        );
    }

    RemoveDestroyedEntities();
}

void Scene::RemoveDestroyedEntities()
{
    std::erase_if(
        m_entities,
        [](const std::unique_ptr<Entity>& entity)
        {
            return
                entity->IsDestroyed();
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
    }
}

void Scene::SyncPhysicsTransforms()
{
    for (auto& entity :
        m_entities)
    {
        if (!entity->active)
        {
            continue;
        }

        entity->
            SyncPhysicsTransform();
    }
}

void Scene::CollectDebugStats(
    DebugStats& stats) const
{
}