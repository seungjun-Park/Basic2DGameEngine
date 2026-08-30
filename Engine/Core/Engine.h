#pragma once

#include <memory>

class WinWindow;
class IRenderer;
class SpriteRenderer;
class ResourceManager;
class Scene;
class Camera;


class Engine
{
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool Initialize(
        WinWindow& window
    );

    void Update();

    void Render();

    void SetScene(
        std::unique_ptr<Scene> scene
    );

    ResourceManager& GetResourceManager();
    Camera& GetCamera();

private:
    std::unique_ptr<IRenderer>
        m_renderer;

    std::unique_ptr<SpriteRenderer>
        m_spriteRenderer;

    std::unique_ptr<ResourceManager>
        m_resourceManager;

    std::unique_ptr<Scene>
        m_scene;

    std::unique_ptr<Camera>
        m_camera;
};