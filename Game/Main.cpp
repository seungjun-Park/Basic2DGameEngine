#include "Engine/Core/Application.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/ProjectConfig.h"

#include "Engine/Serialization/ProjectConfigSerializer.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Physics/CollisionEvents.h"
#include "Engine/Audio/AudioSystem.h"

#include "Game/GameScene.h"

#include <memory>

namespace
{
    constexpr wchar_t
        ProjectConfigPath[] =
        L"project.json";
}

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int)
{
    auto projectConfig =
        ProjectConfigSerializer::Load(
            ProjectConfigPath
        );

    if (!projectConfig)
    {
        MessageBoxW(
            nullptr,
            L"Failed to load project.json.",
            L"Basic2DGameEngine",
            MB_OK |
            MB_ICONERROR
        );

        return -1;
    }


    Application application;

    if (!application.Initialize(
        hInstance,
        projectConfig->engine))
    {
        MessageBoxW(
            nullptr,
            L"Application initialization failed.",
            projectConfig->
            engine.
            windowTitle.c_str(),
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

    return application.Run(
        *projectConfig,
        ProjectConfigPath
    );
}