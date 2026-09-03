#include "Scene.h"

#include "Entity.h"
#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Debug/DebugRenderer.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Physics/PhysicsUnits.h"
#include "Engine/Animation/Animator.h"

#include <algorithm>
#include <atomic>

Scene::~Scene()
{
    ClearEntities();
}

EntityHandle Scene::AllocateEntityHandle()
{
    static std::atomic<
        EntityHandle::ValueType
    > nextValue
    {
        1
    };

    const EntityHandle::ValueType value =
        nextValue.fetch_add(
            1,
            std::memory_order_relaxed
        );

    return EntityHandle
    {
        value
    };
}

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

        if (entity->animator)
        {
            entity->animator->Update(
                deltaTime
            );
        }
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
        [this](
            const std::unique_ptr<Entity>& entity)
        {
            if (!entity->IsDestroyed())
            {
                return false;
            }

            const EntityHandle handle =
                entity->GetHandle();

            if (handle.IsValid())
            {
                m_entityLookup.erase(
                    handle.value
                );
            }

            return true;
        }
    );
}

void Scene::DebugRender(
    SpriteRenderer& renderer,
    DebugRenderer& debugRenderer)
{
    for (auto& entity :
        m_entities)
    {
        if (!entity->active)
        {
            continue;
        }

        if (!entity->physicsBody)
        {
            continue;
        }

        if (!entity->physicsBody->
            IsValid())
        {
            continue;
        }

        const b2ShapeId shapeId =
            entity->physicsBody->
            GetShapeId();

        if (B2_IS_NULL(
            shapeId))
        {
            continue;
        }

        if (!b2Shape_IsValid(
            shapeId))
        {
            continue;
        }

        //
        // 실제 Box2D simulation에 등록된
        // shape의 world-space AABB를 가져온다.
        //

        const b2AABB aabb =
            b2Shape_GetAABB(
                shapeId
            );

        const float left =
            PhysicsUnits::ToPixels(
                aabb.lowerBound.x
            );

        const float top =
            PhysicsUnits::ToPixels(
                aabb.lowerBound.y
            );

        const float right =
            PhysicsUnits::ToPixels(
                aabb.upperBound.x
            );

        const float bottom =
            PhysicsUnits::ToPixels(
                aabb.upperBound.y
            );

        const DirectX::XMFLOAT2 center
        {
            (left + right) * 0.5f,
            (top + bottom) * 0.5f
        };

        const DirectX::XMFLOAT2 size
        {
            right - left,
            bottom - top
        };

        debugRenderer.DrawRect(
            renderer,
            center,
            size,
            2.0f
        );
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
    DebugStats& stats
) const
{

}

Entity* Scene::ResolveEntity(
    EntityHandle handle)
{
    if (!handle.IsValid())
    {
        return nullptr;
    }

    const auto it =
        m_entityLookup.find(
            handle.value
        );

    if (it == m_entityLookup.end())
    {
        return nullptr;
    }

    Entity* entity =
        it->second;

    if (!entity)
    {
        return nullptr;
    }

    if (entity->IsDestroyed())
    {
        return nullptr;
    }

    return entity;
}

const Entity* Scene::ResolveEntity(
    EntityHandle handle) const
{
    if (!handle.IsValid())
    {
        return nullptr;
    }

    const auto it =
        m_entityLookup.find(
            handle.value
        );

    if (it == m_entityLookup.end())
    {
        return nullptr;
    }

    const Entity* entity =
        it->second;

    if (!entity)
    {
        return nullptr;
    }

    if (entity->IsDestroyed())
    {
        return nullptr;
    }

    return entity;
}

bool Scene::IsEntityAlive(
    EntityHandle handle) const
{
    return
        ResolveEntity(handle) != nullptr;
}

void Scene::ClearEntities()
{
    //
    // Handle resolution부터 차단한 뒤
    // 실제 Entity를 파괴한다.
    //
    m_entityLookup.clear();

    m_entities.clear();
}