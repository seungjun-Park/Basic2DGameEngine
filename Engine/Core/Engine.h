#pragma once

#include <memory>
#include "Engine/Debug/DebugStats.h"
#include "Engine/Debug/CpuProfiler.h"
#include "EngineConfig.h"

class WinWindow;
class IRenderer;
class SpriteRenderer;
class ResourceManager;
class Scene;
class Camera;
class DebugRenderer;
class PhysicsSystem;
class RenderQueue;
class EventBus;


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

    void FixedUpdate(
        float fixedDeltaTime
    );

    void Update(
        float deltaTime
    );

    void LateUpdate(
        float deltaTime
    );

    void Render(
        bool vsync
    );

    void Resize(
        int width,
        int height
    );

    void SetInterpolationAlpha(
        float alpha
    );

    float GetInterpolationAlpha() const;

    DebugStats& GetDebugStats();

    void SetScene(
        std::unique_ptr<Scene> scene
    );

    void BeginProfileFrame();

    void EndProfileFrame();

    ResourceManager& GetResourceManager();

    Camera& GetCamera();

    PhysicsSystem&
        GetPhysicsSystem();

    EventBus&
        GetEventBus();

private:
    std::unique_ptr<Scene>
        m_scene;

    std::unique_ptr<EventBus>
        m_eventBus;

    std::unique_ptr<Camera>
        m_camera;

    std::unique_ptr<PhysicsSystem>
        m_physicsSystem;

    std::unique_ptr<RenderQueue>
        m_renderQueue;

    std::unique_ptr<DebugRenderer>
        m_debugRenderer;

    std::unique_ptr<IRenderer>
        m_renderer;

    std::unique_ptr<SpriteRenderer>
        m_spriteRenderer;

    std::unique_ptr<ResourceManager>
        m_resourceManager;

    EngineConfig m_config;

    DebugStats m_debugStats;
    CpuProfiler
        m_cpuProfiler;
    bool m_showDebug = true;

    float m_interpolationAlpha = 0.0f;
};