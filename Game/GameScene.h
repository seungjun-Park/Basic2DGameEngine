#pragma once

#include "Engine/Scene/Scene.h"

#include <memory>

class ResourceManager;
class Player;
class Camera;
class PhysicsSystem;
class TileMap;
class TileMapRenderer;
class RenderQueue;

class GameScene :
    public Scene
{
public:
    explicit GameScene(
        ResourceManager& resources,
        Camera& camera,
        PhysicsSystem& physics
    );

    ~GameScene() override;

    void Initialize() override;

    void Update(
        float deltaTime
    ) override;

    void LateUpdate(
        float deltaTime
    ) override;

    void SubmitRender(
        RenderQueue& renderQueue
    ) override;

    void CollectDebugStats(
        DebugStats& stats
    ) const override;

private:
    ResourceManager& m_resources;

    Player* m_player =
        nullptr;

    Camera& m_camera;

    PhysicsSystem& m_physics;

    TileMap* m_tileMap =
        nullptr;

    std::unique_ptr<TileMapRenderer>
        m_tileMapRenderer;
};