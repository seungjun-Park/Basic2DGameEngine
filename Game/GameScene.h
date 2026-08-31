#pragma once

#include "Engine/Scene/Scene.h"

class ResourceManager;
class Player;
class Camera;
class PhysicsSystem;

class GameScene :
    public Scene
{
public:
    explicit GameScene(
        ResourceManager& resources,
        Camera& camera,
        PhysicsSystem& physics
    );

    void Initialize() override;

    void Update(
        float deltaTime
    ) override;

    void LateUpdate(
        float deltaTime
    ) override;

private:
    ResourceManager& m_resources;
    Player* m_player = nullptr;
    Camera& m_camera;
    PhysicsSystem& m_physics;
};