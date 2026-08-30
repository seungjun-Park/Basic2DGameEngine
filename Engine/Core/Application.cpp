#include "Application.h"

#include "Engine.h"
#include "Time.h"

#include "Engine/Platform/Windows/WinWindow.h"
#include "Engine/Platform/Windows/WinInput.h"

Application::Application() = default;

Application::~Application()
{
    if (m_comInitialized)
    {
        CoUninitialize();
    }
}

bool Application::Initialize(
    HINSTANCE hInstance)
{
    m_window =
        std::make_unique<WinWindow>();

    if (!m_window->Initialize(
        hInstance,
        1280,
        720,
        L"Basic2DGameEngine"))
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

    HRESULT hr =
        CoInitializeEx(
            nullptr,
            COINIT_MULTITHREADED
        );

    if (FAILED(hr))
    {
        return false;
    }

    m_comInitialized = true;

    WinInput::Initialize();

    return true;
}

int Application::Run()
{
    while (m_window->ProcessMessages())
    {
        Time::Tick();

        WinInput::Update();

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

Engine& Application::GetEngine()
{
    return *m_engine;
}