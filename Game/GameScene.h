#pragma once

#include "Engine/Event/EventBus.h"
#include "Engine/Scene/Scene.h"
#include "CharacterAnimation.h"
#include "Engine/Audio/AudioPlaybackHandle.h"

#include <memory>
#include <string>

struct CollisionEnterEvent;
struct CollisionExitEvent;

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
struct EnemyDefeatedEvent;

class GameScene :
    public Scene
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

    void CollectDebugStats(
        DebugStats& stats
    ) const override;

    void DebugRender(
        SpriteRenderer& renderer,
        DebugRenderer& debugRenderer
    ) override;

    void DrawGui() override;

    bool SaveSerializedEntities(
        const std::wstring& path
    );

private:
    bool LoadEnemyAnimations();
    
    bool LoadSerializedEntities(
        const std::wstring& path
    );

    void SubscribeCollisionEvents();

    void HandleCollisionEnterEvent(
        const CollisionEnterEvent& event
    );

    void HandleCollisionExitEvent(
        const CollisionExitEvent& event
    );

    bool IsPlayerCollision(
        EntityHandle entityA,
        EntityHandle entityB
    ) const noexcept;

    void DefeatEnemy(
        Enemy& enemy
    );

    bool InitializeGameplayAudio();

    void StopGameplayAudio() noexcept;

    void SubscribeGameplayAudioEvents();

    void HandleEnemyDefeatedAudio(
        const EnemyDefeatedEvent& event
    );

private:
    ResourceManager& m_resources;

    Player* m_player =
        nullptr;

    Camera& m_camera;

    PhysicsSystem& m_physics;

    EventBus& m_events;
    
    AudioSystem&
        m_audio;

    AudioClip*
        m_enemyDefeatSfx = nullptr;

    AudioClip*
        m_bgmClip = nullptr;

    AudioPlaybackHandle
        m_bgmHandle{};

    EntityHandle m_playerHandle{};

    EventSubscription
        m_collisionEnterSubscription;

    EventSubscription
        m_collisionExitSubscription;

    EventSubscription
        m_enemyDefeatedAudioSubscription;

    TileMap* m_tileMap =
        nullptr;

    std::unique_ptr<TileMapRenderer>
        m_tileMapRenderer;

    std::unique_ptr<TileMapCollider>
        m_tileMapCollider;

    CharacterAnimationSet
        m_enemyAnimations;
};