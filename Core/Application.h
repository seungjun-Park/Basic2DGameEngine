#pragma once

#include <Windows.h>
#include <memory>

class WinWindow;
class Engine;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool Initialize(
        HINSTANCE hInstance
    );

    int Run();

private:
    void Update();
    void Render();

private:
    std::unique_ptr<WinWindow> m_window;
    std::unique_ptr<Engine> m_engine;
};