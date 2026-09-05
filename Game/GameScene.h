#pragma once

#include "CharacterAnimation.h"

#include "Engine/Audio/AudioPlaybackHandle.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Serialization/SceneData.h"
#include "Engine/Tile/ITileMapRuntimeTarget.h"

#include <memory>
#include <string>
#include <vector>

struct CollisionEnterEvent;
struct CollisionExitEvent;
struct EnemyDefeatedEvent;
struct TileMapData;

class ResourceManager;
class Player;
class Camera;
class PhysicsSystem;
class TileMap;
class TileMapRenderer;
class RenderQueue;
class TileMapCollider;
class Enemy;
class AudioSystem;
class AudioClip;

class GameScene :
    public Scene,
    public ITileMapRuntimeTarget
{
public:
    explicit GameScene(
        ResourceManager& resources,
        Camera& camera,
        PhysicsSystem& physics,
        EventBus& events,
        AudioSystem& audio
    );

    ~GameScene() override;

    void Initialize() override;

    void Update(
        float deltaTime
    ) override;

    void LateUpdate(
        float deltaTime
    ) override;

    void SubmitRender(
        RenderQueue& renderQueue
    ) override;

    void DebugRender(
        SpriteRenderer& renderer,
        DebugRenderer& debugRenderer
    ) override;

    void DrawGuiContents() override;

    bool SaveSerializedEntities(
        const std::wstring& path
    );

    [[nodiscard]]
    bool IsUsingTileMap(
        const std::wstring& path
    ) const noexcept override;

    bool ApplyTileMapData(
        const std::wstring& path,
        const TileMapData& data
    ) override;

    void CollectDebugStats(
        DebugStats& stats
    ) const override;

private:
    bool LoadTileMap(
        const SceneData& sceneData
    );

    bool LoadEnemyAnimations(
        const SceneData& sceneData
    );

    bool LoadSerializedEntities(
        const std::wstring& path
    );

    bool InitializeGameplayAudio();

    void StopGameplayAudio() noexcept;

    void DefeatEnemy(
        Enemy& enemy
    );

private:
    void SubscribeCollisionEvents();

    void SubscribeGameplayAudioEvents();

private:
    void HandleCollisionEnterEvent(
        const CollisionEnterEvent& event
    );

    void HandleCollisionExitEvent(
        const CollisionExitEvent& event
    );

    void HandleEnemyDefeatedAudio(
        const EnemyDefeatedEvent& event
    );

private:
    bool IsPlayerCollision(
        EntityHandle entityA,
        EntityHandle entityB
    ) const noexcept;

private:
    ResourceManager& m_resources;

    Player* m_player =
        nullptr;

    Camera& m_camera;

    PhysicsSystem& m_physics;

    EventBus& m_events;

    AudioSystem& m_audio;

    AudioClip* m_enemyDefeatSfx =
        nullptr;

    AudioClip* m_bgmClip =
        nullptr;

    AudioPlaybackHandle
        m_bgmHandle{};

    EntityHandle
        m_playerHandle{};

    EventSubscription
        m_collisionEnterSubscription;

    EventSubscription
        m_collisionExitSubscription;

    EventSubscription
        m_enemyDefeatedAudioSubscription;

    TileMap* m_tileMap =
        nullptr;

    std::wstring
        m_tileMapPath;

    std::unique_ptr<TileMapRenderer>
        m_tileMapRenderer;

    std::unique_ptr<TileMapCollider>
        m_tileMapCollider;

    CharacterAnimationSet
        m_enemyAnimations;

    std::vector<SerializedAnimationBinding>
        m_animationBindings;
};