#include "Engine/Core/Application.h"
#include "Engine/Core/Engine.h"
#include "Game/GameScene.h"

#include <Windows.h>

#include <memory>
#include <utility>

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int)
{
    EngineConfig config;

    config.windowWidth = 1280;
    config.windowHeight = 960;

    config.vsync = false;

    config.targetFPS = 0;

    config.fixedUpdateHz = 60.0f;

    config.maxFixedSteps = 5;

    config.maxDeltaTime = 0.1f;

    config.pauseWhenUnfocused = true;

    Application application;

    if (!application.Initialize(
        hInstance,
        config))
    {
        MessageBoxW(
            nullptr,
            L"Application initialization failed.",
            L"Dobi2D",
            MB_OK |
            MB_ICONERROR
        );

        return -1;
    }

    Engine& engine =
        application.GetEngine();

    auto gameScene =
        std::make_unique<GameScene>(
            engine.GetResourceManager(),
            engine.GetCamera(),
            engine.GetPhysicsSystem(),
            engine.GetEventBus(),
            engine.GetAudioSystem()
        );

    engine.SetScene(
        std::move(gameScene)
    );

    return application.Run();
}