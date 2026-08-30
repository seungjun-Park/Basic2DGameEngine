#pragma once

#include <memory>

class WinWindow;
class IRenderer;

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

private:
    std::unique_ptr<IRenderer> m_renderer;
};