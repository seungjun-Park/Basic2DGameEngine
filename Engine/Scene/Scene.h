#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

#include "Engine/Scene/EntityHandle.h"

class Entity;
class SpriteRenderer;
class DebugRenderer;
class RenderQueue;
struct DebugStats;

class Scene
{
public:
    Scene() = default;
    virtual ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    virtual void Initialize()
    {
    }

    virtual void FixedUpdate(
        float fixedDeltaTime
    );

    virtual void Update(
        float deltaTime
    );

    virtual void LateUpdate(
        float deltaTime
    );

    virtual void SubmitRender(
        RenderQueue& renderQueue
    );

    virtual void DebugRender(
        SpriteRenderer& renderer,
        DebugRenderer& debugRenderer
    );

    std::size_t GetEntityCount() const
    {
        return m_entities.size();
    }

    Entity* ResolveEntity(
        EntityHandle handle
    );

    const Entity* ResolveEntity(
        EntityHandle handle
    ) const;

    bool IsEntityAlive(
        EntityHandle handle
    ) const;

    template<typename T, typename... Args>
    T* CreateEntity(
        Args&&... args)
    {
        static_assert(
            std::is_base_of_v<Entity, T>,
            "T must inherit from Entity"
            );

        auto entity =
            std::make_unique<T>(
                std::forward<Args>(args)...
            );

        T* result =
            entity.get();

        const EntityHandle handle =
            AllocateEntityHandle();

        entity->SetHandle(
            handle
        );

        m_entities.emplace_back(
            std::move(entity)
        );

        m_entityLookup.emplace(
            handle.value,
            result
        );

        return result;
    }

    virtual void SyncPhysicsTransforms();

    virtual void CollectDebugStats(
        DebugStats& stats
    ) const;

protected:
    void RemoveDestroyedEntities();

    void ClearEntities();

private:

    static EntityHandle
        AllocateEntityHandle();


protected:
    std::vector<
        std::unique_ptr<Entity>
    > m_entities;

private:

    std::unordered_map<
        EntityHandle::ValueType,
        Entity*
    > m_entityLookup;
};
