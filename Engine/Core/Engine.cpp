#include "Engine.h"

#include "Time.h"

#include "Engine/Platform/Windows/WinWindow.h"
#include "Engine/Renderer/IRenderer.h"
#include "Engine/Renderer/DX11Renderer.h"
#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Camera.h"

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

    m_camera = std::make_unique<Camera>();

    m_camera->Initialize(
        static_cast<float>(
            window.GetWidth()
            ),
        static_cast<float>(
            window.GetHeight()
            )
    );

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

void Engine::Update()
{
    if (!m_scene)
        return;

    m_scene->Update(
        Time::DeltaTime()
    );
}

void Engine::Render()
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

        m_spriteRenderer->End();
    }

    m_renderer->EndFrame();
}

Camera& Engine::GetCamera()
{
    return *m_camera;
}