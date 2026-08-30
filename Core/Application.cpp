#include "Application.h"

#include "Engine.h"
#include "Time.h"

#include "Platform/Windows/WinWindow.h"

Application::Application() = default;

Application::~Application() = default;

bool Application::Initialize(
    HINSTANCE hInstance)
{
    m_window =
        std::make_unique<WinWindow>();

    if (!m_window->Initialize(
        hInstance,
        1280,
        720,
        L"Dobi2D - Day 1"))
    {
        return false;
    }

    Time::Initialize();

    m_engine =
        std::make_unique<Engine>();

    if (!m_engine->Initialize(
        *m_window))
    {
        return false;
    }

    return true;
}

int Application::Run()
{
    while (m_window->ProcessMessages())
    {
        Time::Tick();

        Update();
        Render();
    }

    return 0;
}

void Application::Update()
{
    m_engine->Update();
}

void Application::Render()
{
    m_engine->Render();
}