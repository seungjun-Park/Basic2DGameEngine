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
class AudioSystem;
class GuiSystem;


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

    void BeginGuiFrame();
    void BeginProfileFrame();
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
    void EndProfileFrame();

    
    void Resize(
        int width,
        int height
    );
    void SetScene(
        std::unique_ptr<Scene> scene
    );
    void SetInterpolationAlpha(
        float alpha
    );
    void SetDebugVisible(
        bool visible
    );

    float GetInterpolationAlpha() const;
    ResourceManager& GetResourceManager();
    Camera& GetCamera();
    PhysicsSystem&
        GetPhysicsSystem();
    EventBus&
        GetEventBus();
    AudioSystem&
        GetAudioSystem();
    DebugStats& GetDebugStats();

    bool IsGuiVisible() const;

private:
    std::unique_ptr<Scene>
        m_scene;
    std::unique_ptr<EventBus>
        m_eventBus;
    std::unique_ptr<Camera>
        m_camera;
    std::unique_ptr<PhysicsSystem>
        m_physicsSystem;
    std::unique_ptr<AudioSystem>
        m_audioSystem;
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
    std::unique_ptr<GuiSystem> m_guiSystem;

    EngineConfig m_config;

    DebugStats m_debugStats;
    CpuProfiler
        m_cpuProfiler;
    bool m_showDebug = true;
    bool m_showGui = true;

    float m_interpolationAlpha = 0.0f;
};