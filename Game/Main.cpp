#include "Engine/Core/Application.h"
#include "Engine/Core/Engine.h"
#include "Engine/Resource/ResourceManager.h"

#include "Game/GameScene.h"

#include <memory>

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int)
{
    Application application;

    if (!application.Initialize(
        hInstance))
    {
        MessageBoxW(
            nullptr,
            L"Application initialization failed.",
            L"Dobi2D",
            MB_OK | MB_ICONERROR
        );

        return -1;
    }

    Engine& engine =
        application.GetEngine();

    auto gameScene =
        std::make_unique<GameScene>(
            engine.GetResourceManager(),
            engine.GetCamera()
        );

    engine.SetScene(
        std::move(gameScene)
    );

    return application.Run();
}