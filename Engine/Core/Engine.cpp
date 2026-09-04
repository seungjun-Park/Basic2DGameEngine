#include "Engine.h"

#include "Time.h"

#include "Engine/Platform/Windows/WinWindow.h"
#include "Engine/Renderer/IRenderer.h"
#include "Engine/Renderer/DX11Renderer.h"
#include "Engine/Renderer/SpriteRenderer.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Debug/DebugRenderer.h"
#include "Engine/Platform/Windows/WinInput.h"
#include "Engine/Physics/PhysicsSystem.h"
#include "Engine/Renderer/RenderQueue.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Audio/AudioSystem.h"
#include "Engine/GUI/GuiSystem.h"
#include "Engine/GUI/EngineGui.h"

#include <algorithm>

Engine::Engine() = default;

Engine::~Engine()
{
    // Scene을 가장 먼저 제거한다.
    //
    // Entity
    // → PhysicsBody
    // → b2DestroyBody()
    //
    // 가 PhysicsSystem이 살아있는 동안
    // 수행되어야 한다.
    m_scene.reset();

    // DebugRenderer는 ResourceManager의
    // white texture를 raw pointer로 참조하므로
    // ResourceManager보다 먼저 제거한다.
    m_debugRenderer.reset();

    // Frame-local command pointer를 보유하는
    // RenderQueue도 더 이상 필요하지 않다.
    m_renderQueue.reset();

    // 모든 Entity PhysicsBody가 제거된 뒤
    // Box2D World를 제거한다.
    m_physicsSystem.reset();

    m_camera.reset();

    m_audioSystem.reset();

    // Texture / Tileset / TileMap을
    // GPU device가 살아있는 동안 제거한다.
    m_resourceManager.reset();

    // SpriteRenderer의 D3D resource도
    // DX11Renderer device보다 먼저 제거한다.
    m_spriteRenderer.reset();

    m_guiSystem.reset();

    // Device / Context / SwapChain은 마지막.
    m_renderer.reset();

    //
    // 모든 event publisher / subscriber가
    // 제거된 뒤 EventBus를 마지막으로 파괴.
    //
    m_eventBus.reset();
}

bool Engine::Initialize(
    WinWindow& window)
{
    m_eventBus =
        std::make_unique<EventBus>();

    m_renderQueue =
        std::make_unique<RenderQueue>();

    m_renderer =
        std::make_unique<DX11Renderer>();

    if (!m_renderer->Initialize(
        window.GetHandle(),
        window.GetWidth(),
        window.GetHeight()))
    {
        return false;
    }

    auto* dx11 =
        static_cast<DX11Renderer*>(
            m_renderer.get()
            );

    m_spriteRenderer =
        std::make_unique<SpriteRenderer>();

    if (!m_spriteRenderer->Initialize(
        *dx11,
        window.GetWidth(),
        window.GetHeight()))
    {
        return false;
    }

    m_resourceManager =
        std::make_unique<ResourceManager>();

    if (!m_resourceManager->Initialize(
        *dx11))
    {
        return false;
    }

    m_audioSystem =
        std::make_unique<
        AudioSystem
        >();

    if (!m_audioSystem->
        Initialize())
    {
        OutputDebugStringA(
            "[Engine] Failed to initialize "
            "AudioSystem.\n"
        );

        return false;
    }

    m_physicsSystem =
        std::make_unique<PhysicsSystem>();

    if (!m_physicsSystem->Initialize())
    {
        return false;
    }

    m_camera = std::make_unique<Camera>();

    m_camera->Initialize(
        static_cast<float>(
            window.GetWidth()
            ),
        static_cast<float>(
            window.GetHeight()
            )
    );

    m_debugRenderer =
        std::make_unique<DebugRenderer>();

    Texture* whiteTexture =
        m_resourceManager->LoadTexture(
            L"Engine/Assets/Textures/white.png"
        );

    if (!m_debugRenderer->Initialize(
        whiteTexture))
    {
        return false;
    }

    m_guiSystem = std::make_unique<GuiSystem>();

    if (!m_guiSystem->Initialize(
        window.GetHandle(),
        dx11->GetDevice(),
        dx11->GetContext()))
    {
        return false;
    }

    return true;
}

void Engine::SetScene(
    std::unique_ptr<Scene> scene)
{
    m_scene =
        std::move(scene);

    if (m_scene)
    {
        m_scene->Initialize();
    }
}

ResourceManager&
Engine::GetResourceManager()
{
    return *m_resourceManager;
}

EventBus&
Engine::GetEventBus()
{
    return
        *m_eventBus;
}

void Engine::FixedUpdate(
    float fixedDeltaTime)
{
    ScopedCpuProfile profileScope(
        m_cpuProfiler,
        CpuProfileZone::FixedUpdate
    );

    if (!m_scene)
    {
        return;
    }

    {
        ScopedCpuProfile subsystemScope(
            m_cpuProfiler,
            CpuProfileZone::FixedUpdate
        );
        // 1.
        // 게임 로직이 velocity / force를 Physics에 전달
        m_scene->FixedUpdate(
            fixedDeltaTime
        );
    }

    {
        ScopedCpuProfile subsystemScope(
            m_cpuProfiler,
            CpuProfileZone::FixedUpdate
        );
        // 2.
        // 실제 Box2D simulation
        m_physicsSystem->Step(
            fixedDeltaTime
        );
    }

    {
        ScopedCpuProfile subsystemScope(
            m_cpuProfiler,
            CpuProfileZone::FixedUpdate
        );
        // 3.
        // Box2D 결과를 render Transform으로 반영
        m_scene->
            SyncPhysicsTransforms();
    }

    {
        ScopedCpuProfile subsystemScope(
            m_cpuProfiler,
            CpuProfileZone::FixedUpdate
        );
        // 4.
        // contact event 전달
        m_physicsSystem->
            DispatchContactEvents(
                *m_scene,
                *m_eventBus
            );
    }
}

void Engine::Update(
    float deltaTime)
{
    ScopedCpuProfile profileScope(
        m_cpuProfiler,
        CpuProfileZone::Update
    );

    if (m_audioSystem)
    {
        m_audioSystem->
            Update();
    }

    if (WinInput::IsRawKeyPressed(
        VK_F1))
    {
        m_showDebug =
            !m_showDebug;
    }

    if (WinInput::IsRawKeyPressed(VK_F3))
    {
        m_showGui = !m_showGui;

        if (!m_showGui)
        {
            WinInput::SetCaptureState(
                false,
                false);
        }
    }

    if (!m_scene)
    {
        return;
    }

    m_scene->Update(
        deltaTime
    );
}

void Engine::LateUpdate(
    float deltaTime)
{
    ScopedCpuProfile profileScope(
        m_cpuProfiler,
        CpuProfileZone::LateUpdate
    );

    if (!m_scene)
    {
        return;
    }

    m_scene->LateUpdate(
        deltaTime
    );
}

void Engine::Render(
    bool vsync)
{
    {
        ScopedCpuProfile profileScope(
            m_cpuProfiler,
            CpuProfileZone::RenderCpu
        );

        m_renderer->BeginFrame();

        if (m_scene)
        {
            m_spriteRenderer->SetCamera(
                *m_camera
            );

            m_renderQueue->Clear();

            {
                ScopedCpuProfile subsystemScope(
                    m_cpuProfiler,
                    CpuProfileZone::
                    RenderSubmit
                );

                m_scene->SubmitRender(
                    *m_renderQueue
                );
            }

            {
                ScopedCpuProfile subsystemScope(
                    m_cpuProfiler,
                    CpuProfileZone::
                    RenderSort
                );

                m_renderQueue->Sort();
            }

            m_spriteRenderer->Begin();

            {
                ScopedCpuProfile subsystemScope(
                    m_cpuProfiler,
                    CpuProfileZone::
                    RenderExecute
                );

                m_renderQueue->Execute(
                    *m_spriteRenderer
                );
            }

#ifdef _DEBUG

            if (m_showDebug)
            {
                ScopedCpuProfile subsystemScope(
                    m_cpuProfiler,
                    CpuProfileZone::
                    DebugRender
                );

                m_scene->DebugRender(
                    *m_spriteRenderer,
                    *m_debugRenderer
                );
            }

#endif

            m_spriteRenderer->End();

            m_debugStats.entityCount =
                static_cast<int>(
                    m_scene->GetEntityCount()
                    );

            m_debugStats.renderCommands =
                static_cast<int>(
                    m_renderQueue->
                    GetCommandCount()
                    );

            m_debugStats.drawCalls =
                m_spriteRenderer->
                GetDrawCallCount();

            const RenderBatchStats&
                batchStats =
                m_renderQueue->
                GetBatchStats();

            m_debugStats.renderBatches =
                static_cast<std::uint32_t>(
                    batchStats.batchCount
                    );

            m_debugStats.batchedRenderCommands =
                static_cast<std::uint32_t>(
                    batchStats.
                    batchedCommandCount
                    );

            m_debugStats.maxBatchSize =
                static_cast<std::uint32_t>(
                    batchStats.maxBatchSize
                    );

            m_debugStats.singleCommandBatches =
                static_cast<std::uint32_t>(
                    batchStats.
                    singleCommandBatchCount
                    );

            m_debugStats.batchBoundaries =
                static_cast<std::uint32_t>(
                    batchStats.
                    batchBoundaryCount
                    );

            m_debugStats.textureBatchBoundaries =
                static_cast<std::uint32_t>(
                    batchStats.
                    textureBoundaryCount
                    );

            m_debugStats.blendBatchBoundaries =
                static_cast<std::uint32_t>(
                    batchStats.
                    blendBoundaryCount
                    );

            m_debugStats.layerBatchBoundaries =
                static_cast<std::uint32_t>(
                    batchStats.
                    layerBoundaryCount
                    );

            m_debugStats.invalidRenderCommands =
                static_cast<std::uint32_t>(
                    batchStats.
                    invalidCommandCount
                    );

            m_debugStats.ResetTileStats();

            m_scene->CollectDebugStats(
                m_debugStats
            );
        }

        if (m_guiSystem &&
            m_guiSystem->IsInitialized())
        {
            if (m_showGui)
            {
                if (m_audioSystem)
                {
                    EngineGui::DrawAudioSettings(
                        *m_audioSystem);
                }

                if (m_scene)
                {
                    m_scene->DrawGui();
                }
            }

            m_guiSystem->Render();
        }
    }

    {
        ScopedCpuProfile profileScope(
            m_cpuProfiler,
            CpuProfileZone::Present
        );

        m_renderer->EndFrame(
            vsync
        );
    }
}

void Engine::Resize(
    int width,
    int height)
{
    if (width <= 0 ||
        height <= 0)
    {
        return;
    }

    m_renderer->Resize(
        width,
        height
    );

    m_camera->Resize(
        static_cast<float>(
            width
            ),
        static_cast<float>(
            height
            )
    );
}

void Engine::SetInterpolationAlpha(
    float alpha)
{
    m_interpolationAlpha =
        std::clamp(
            alpha,
            0.0f,
            1.0f
        );
}

float Engine::GetInterpolationAlpha() const
{
    return m_interpolationAlpha;
}

Camera& Engine::GetCamera()
{
    return *m_camera;
}

DebugStats&
Engine::GetDebugStats()
{
    return m_debugStats;
}

PhysicsSystem&
Engine::GetPhysicsSystem()
{
    return *m_physicsSystem;
}

void Engine::BeginProfileFrame()
{
    m_cpuProfiler.BeginFrame();
}


void Engine::EndProfileFrame()
{
    m_cpuProfiler.EndFrame();

    const CpuProfileSnapshot&
        profile =
        m_cpuProfiler.
        GetLatestSnapshot();

    m_debugStats.fixedUpdateCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                FixedUpdate
            )
            );

    m_debugStats.
        fixedStepAverageCpuMs =
        static_cast<float>(
            profile.
            GetAverageSampleMs(
                CpuProfileZone::
                FixedUpdate
            )
            );

    m_debugStats.profiledFixedSteps =
        profile.GetSampleCount(
            CpuProfileZone::
            FixedUpdate
        );

    m_debugStats.updateCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::Update
            )
            );

    m_debugStats.lateUpdateCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                LateUpdate
            )
            );

    m_debugStats.renderCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                RenderCpu
            )
            );

    m_debugStats.presentMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                Present
            )
            );

    m_debugStats.engineCpuWorkMs =
        static_cast<float>(
            profile.
            GetEngineCpuWorkMs()
            );
    m_debugStats.sceneFixedCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                SceneFixedUpdate
            )
            );

    m_debugStats.physicsStepCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                PhysicsStep
            )
            );

    m_debugStats.physicsSyncCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                PhysicsSync
            )
            );

    m_debugStats.contactDispatchCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                ContactDispatch
            )
            );


    m_debugStats.renderSubmitCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                RenderSubmit
            )
            );

    m_debugStats.renderSortCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                RenderSort
            )
            );

    m_debugStats.renderExecuteCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                RenderExecute
            )
            );

    m_debugStats.debugRenderCpuMs =
        static_cast<float>(
            profile.GetTotalMs(
                CpuProfileZone::
                DebugRender
            )
            );
    const float fixedChildrenMs =
        m_debugStats.sceneFixedCpuMs +
        m_debugStats.physicsStepCpuMs +
        m_debugStats.physicsSyncCpuMs +
        m_debugStats.contactDispatchCpuMs;

    m_debugStats.fixedOverheadCpuMs =
        std::max(
            0.0f,
            m_debugStats.fixedUpdateCpuMs -
            fixedChildrenMs
        );

    const float renderChildrenMs =
        m_debugStats.renderSubmitCpuMs +
        m_debugStats.renderSortCpuMs +
        m_debugStats.renderExecuteCpuMs +
        m_debugStats.debugRenderCpuMs;

    m_debugStats.renderOverheadCpuMs =
        std::max(
            0.0f,
            m_debugStats.renderCpuMs -
            renderChildrenMs
        );

    const CpuProfilerDiagnostics&
        diagnostics =
        m_cpuProfiler.
        GetDiagnostics();


    m_debugStats.profilerHistoryFrames =
        diagnostics.historyCount;

    m_debugStats.cpuWorkAverageMs =
        static_cast<float>(
            diagnostics.
            averageEngineCpuWorkMs
            );

    m_debugStats.cpuWorkMaxMs =
        static_cast<float>(
            diagnostics.
            maxEngineCpuWorkMs
            );

    m_debugStats.cpuWorkMaxFramesAgo =
        diagnostics.
        maxEngineCpuWorkFramesAgo;


    m_debugStats.presentAverageMs =
        static_cast<float>(
            diagnostics.
            averagePresentMs
            );

    m_debugStats.presentMaxMs =
        static_cast<float>(
            diagnostics.
            maxPresentMs
            );


    m_debugStats.cpuSpikeThresholdMs =
        static_cast<float>(
            diagnostics.
            spikeThresholdMs
            );

    m_debugStats.currentCpuSpike =
        diagnostics.
        currentCpuSpike;

    m_debugStats.cpuSpikesInHistory =
        diagnostics.
        cpuSpikesInHistory;

    m_debugStats.latestCpuSpikeFramesAgo =
        diagnostics.
        latestSpikeFramesAgo;


    m_debugStats.peakFrameWorstCpuPhase =
        diagnostics.
        peakFrameWorstCpuPhase;

    m_debugStats.peakFrameWorstCpuPhaseMs =
        static_cast<float>(
            diagnostics.
            peakFrameWorstCpuPhaseMs
            );


    m_debugStats.peakFrameWorstSubsystem =
        diagnostics.
        peakFrameWorstSubsystem;

    m_debugStats.peakFrameWorstSubsystemMs =
        static_cast<float>(
            diagnostics.
            peakFrameWorstSubsystemMs
            );
}

AudioSystem&
Engine::GetAudioSystem()
{
    return
        *m_audioSystem;
}

void Engine::BeginGuiFrame()
{
    if (!m_guiSystem ||
        !m_guiSystem->IsInitialized())
    {
        WinInput::SetCaptureState(
            false,
            false);

        return;
    }

    m_guiSystem->BeginFrame();

    if (!m_showGui)
    {
        WinInput::SetCaptureState(
            false,
            false);

        return;
    }

    WinInput::SetCaptureState(
        m_guiSystem->WantsCaptureKeyboard(),
        m_guiSystem->WantsCaptureMouse());
}