#pragma once

#include "Engine/Scene/Scene.h"

class ResourceManager;
class Player;
class Camera;

class GameScene :
    public Scene
{
public:
    explicit GameScene(
        ResourceManager& resources,
        Camera& camera
    );

    void Initialize() override;

    void Update(
        float deltaTime
    ) override;

private:
    ResourceManager& m_resources;
    Player* m_player = nullptr;
    Camera& m_camera;
};