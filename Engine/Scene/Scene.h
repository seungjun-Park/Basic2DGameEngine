#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class Entity;
class SpriteRenderer;
class DebugRenderer;
class RenderQueue;

class Scene
{
public:
    Scene() = default;
    virtual ~Scene() = default;

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

        m_entities.emplace_back(
            std::move(entity)
        );

        return result;
    }

    virtual void SyncPhysicsTransforms();

protected:
    void RemoveDestroyedEntities();

protected:
    std::vector<
        std::unique_ptr<Entity>
    > m_entities;
};
