#include "Engine/Core/Application.h"
#include "Engine/Core/Engine.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Physics/CollisionEvents.h"
#include "Game/GameplayEvents.h"
#include "Engine/Audio/AudioClip.h"
#include "Game/GameScene.h"

#include <cassert>
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

#if defined(_DEBUG)

    {
        ResourceManager&
            resources =
            engine.GetResourceManager();


        const std::wstring path =
            L"Engine/Assets/Audio/test.wav";


        AudioClip* clipA =
            resources.LoadAudioClip(
                path
            );

        AudioClip* clipB =
            resources.LoadAudioClip(
                path
            );


        assert(clipA);
        assert(clipB);


        //
        // Cache identity.
        //
        assert(
            clipA == clipB
        );


        assert(
            clipA->IsValid()
        );


        const WAVEFORMATEX&
            format =
            clipA->GetFormat();


        assert(
            format.wFormatTag ==
            WAVE_FORMAT_PCM
        );

        assert(
            format.nChannels > 0
        );

        assert(
            format.nSamplesPerSec > 0
        );

        assert(
            format.nBlockAlign > 0
        );

        assert(
            format.nAvgBytesPerSec > 0
        );

        assert(
            format.wBitsPerSample > 0
        );


        assert(
            clipA->GetAudioData() !=
            nullptr
        );

        assert(
            clipA->GetAudioByteCount() >
            0
        );

        assert(
            clipA->GetDurationSeconds() >
            0.0f
        );


        assert(
            clipA->GetAudioByteCount() %
            format.nBlockAlign ==
            0
        );


        OutputDebugStringA(
            "[Phase13-B] "
            "AudioClip validation passed.\n"
        );
    }

#endif

    auto gameScene =
        std::make_unique<GameScene>(
            engine.GetResourceManager(),
            engine.GetCamera(),
            engine.GetPhysicsSystem(),
            engine.GetEventBus()
        );

    engine.SetScene(
        std::move(gameScene)
    );

    return application.Run();
}