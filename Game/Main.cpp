#include "Engine/Core/Application.h"
#include "Engine/Core/Engine.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Physics/CollisionEvents.h"
#include <cassert>

#include "Game/GameScene.h"

#include <memory>

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

    // 0ÀÌ¸י Unlimited
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
            engine.GetPhysicsSystem()
        );

    engine.SetScene(
        std::move(gameScene)
    );

    return application.Run();
}