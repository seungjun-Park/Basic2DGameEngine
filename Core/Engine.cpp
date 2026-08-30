#include "Engine.h"

#include "Platform/Windows/WinWindow.h"
#include "Renderer/IRenderer.h"
#include "Renderer/DX11Renderer.h"

#include "Time.h"

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

    return true;
}

Engine::Engine() = default;

Engine::~Engine() = default;

void Engine::Update()
{
    // Day 1에서는 비워둔다.
    //
    // Day 2 이후:
    // Scene->Update(Time::DeltaTime());
}

void Engine::Render()
{
    m_renderer->BeginFrame();

    // Day 1에서는 Clear만 수행한다.
    //
    // Day 2 이후:
    // SpriteRenderer
    // Camera
    // Scene
    // 등을 이곳에서 호출한다.

    m_renderer->EndFrame();
}