#include "Engine.h"

#include "Time.h"

#include "Engine/Platform/Windows/WinWindow.h"
#include "Engine/Renderer/IRenderer.h"
#include "Engine/Renderer/DX11Renderer.h"
#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Debug/DebugRenderer.h"
#include "Engine/Platform/Windows/WinInput.h"
#include "Engine/Physics/PhysicsSystem.h"

#include <algorithm>

Engine::Engine() = default;

Engine::~Engine() = default;

bool Engine::Initialize(
    WinWindow& window)
{
    m_renderer =
        std::make_unique<DX11Renderer>();

    if (!m_renderer->Initialize(
        window.GetHandle(),
        window.GetWidth(),
        window.GetHeight()))
    {
        return false;
    }

    auto* dx11 =
        static_cast<DX11Renderer*>(
            m_renderer.get()
            );

    m_spriteRenderer =
        std::make_unique<SpriteRenderer>();

    if (!m_spriteRenderer->Initialize(
        *dx11,
        window.GetWidth(),
        window.GetHeight()))
    {
        return false;
    }

    m_resourceManager =
        std::make_unique<ResourceManager>();

    if (!m_resourceManager->Initialize(
        *dx11))
    {
        return false;
    }

    m_physicsSystem =
        std::make_unique<PhysicsSystem>();

    if (!m_physicsSystem->Initialize())
    {
        return false;
    }

    m_camera = std::make_unique<Camera>();

    m_camera->Initialize(
        static_cast<float>(
            window.GetWidth()
            ),
        static_cast<float>(
            window.GetHeight()
            )
    );

    m_debugRenderer =
        std::make_unique<DebugRenderer>();

    Texture* whiteTexture =
        m_resourceManager->LoadTexture(
            L"Engine/Assets/Textures/white.png"
        );

    if (!m_debugRenderer->Initialize(
        whiteTexture))
    {
        return false;
    }

    return true;
}

void Engine::SetScene(
    std::unique_ptr<Scene> scene)
{
    m_scene =
        std::move(scene);

    if (m_scene)
    {
        m_scene->Initialize();
    }
}

ResourceManager&
Engine::GetResourceManager()
{
    return *m_resourceManager;
}

void Engine::FixedUpdate(
    float fixedDeltaTime)
{
    if (!m_scene)
    {
        return;
    }

    // 1.
    // 게임 로직이 velocity / force를 Physics에 전달
    m_scene->FixedUpdate(
        fixedDeltaTime
    );

    // 2.
    // 실제 Box2D simulation
    m_physicsSystem->Step(
        fixedDeltaTime
    );

    // 3.
    // Box2D 결과를 render Transform으로 반영
    m_scene->
        SyncPhysicsTransforms();

    // 4.
    // contact event 전달
    m_physicsSystem->
        DispatchContactEvents();
}

void Engine::Update(
    float deltaTime)
{
    if (WinInput::IsKeyPressed(
        VK_F1))
    {
        m_showDebug =
            !m_showDebug;
    }

    if (!m_scene)
    {
        return;
    }

    m_scene->Update(
        deltaTime
    );
}

void Engine::LateUpdate(
    float deltaTime)
{
    if (!m_scene)
    {
        return;
    }

    m_scene->LateUpdate(
        deltaTime
    );
}

void Engine::Render(
    bool vsync)
{
    m_renderer->BeginFrame();

    if (m_scene)
    {
        m_spriteRenderer->SetCamera(
            *m_camera
        );

        m_spriteRenderer->Begin();

        m_scene->Render(
            *m_spriteRenderer
        );

#ifdef _DEBUG

        if (m_showDebug)
        {
            m_scene->DebugRender(
                *m_spriteRenderer,
                *m_debugRenderer
            );
        }

#endif

        m_spriteRenderer->End();

        m_debugStats.entityCount =
            static_cast<int>(
                m_scene->GetEntityCount()
                );

        m_debugStats.drawCalls =
            m_spriteRenderer->
            GetDrawCallCount();
    }

    m_renderer->EndFrame(
        vsync
    );
}

void Engine::Resize(
    int width,
    int height)
{
    if (width <= 0 ||
        height <= 0)
    {
        return;
    }

    m_renderer->Resize(
        width,
        height
    );

    m_camera->Resize(
        static_cast<float>(
            width
            ),
        static_cast<float>(
            height
            )
    );
}

void Engine::SetInterpolationAlpha(
    float alpha)
{
    m_interpolationAlpha =
        std::clamp(
            alpha,
            0.0f,
            1.0f
        );
}

float Engine::GetInterpolationAlpha() const
{
    return m_interpolationAlpha;
}

Camera& Engine::GetCamera()
{
    return *m_camera;
}

DebugStats&
Engine::GetDebugStats()
{
    return m_debugStats;
}

PhysicsSystem&
Engine::GetPhysicsSystem()
{
    return *m_physicsSystem;
}