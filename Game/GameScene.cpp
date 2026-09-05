#include "GameScene.h"

#include "Player.h"
#include "Enemy.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Physics/PhysicsUnits.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Tile/TileMapRenderer.h"
#include "Engine/Tile/TileMapCollider.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Debug/DebugRenderer.h"
#include "Engine/Animation/AnimationClip.h"

#include "GameEntityFactory.h"
#include "Engine/Serialization/SceneData.h"
#include "Engine/Serialization/SceneSerializer.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Physics/CollisionEvents.h"
#include "GameplayEvents.h"
#include "Engine/Audio/AudioClip.h"
#include "Engine/Audio/AudioSystem.h"
#include "Engine/GUI/EngineGui.h"
#include "Engine/Debug/DebugLog.h"

#include <imgui.h>

#include <optional>
#include <utility>


namespace
{
    constexpr const wchar_t*
        GameplayBgmPath =
        L"Engine/Assets/Audio/bgm.wav";

    constexpr const wchar_t*
        EnemyDefeatSfxPath =
        L"Engine/Assets/Audio/"
        L"enemy_defeat.wav";

    constexpr float
        GameplayBgmVolume =
        0.35f;

    constexpr float
        EnemyDefeatSfxVolume =
        0.70f;
}


GameScene::GameScene(
    ResourceManager& resources,
    Camera& camera,
    PhysicsSystem& physics,
    EventBus& events,
    AudioSystem& audio)
    :
    m_resources(resources),
    m_camera(camera),
    m_physics(physics),
    m_events(events),
    m_audio(audio)
{
}

GameScene::~GameScene()
{
    m_collisionEnterSubscription.Reset();
    m_collisionExitSubscription.Reset();
    m_enemyDefeatedAudioSubscription.Reset();

    StopGameplayAudio();
}

void GameScene::Initialize()
{
    m_tileMap =
        m_resources.LoadTileMap(
            L"Engine/Assets/Maps/testmap.json"
        );


    if (!m_tileMap)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to load TileMap.\n"
        );

        return;
    }

    m_tileMapRenderer =
        std::make_unique<
        TileMapRenderer
        >();

    if (!m_tileMapRenderer->Build(
        *m_tileMap))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to build TileMapRenderer.\n"
        );

        m_tileMapRenderer.reset();

        return;
    }

    m_tileMapCollider =
        std::make_unique<
        TileMapCollider
        >();

    if (!m_tileMapCollider->Build(
        *m_tileMap,
        m_physics))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to build "
            "TileMapCollider.\n"
        );

        m_tileMapCollider.reset();

        return;
    }
    
    if (!LoadEnemyAnimations())
    {
        return;
    }

    if (!LoadSerializedEntities(
        L"Engine/Assets/Scenes/"
        L"test_scene.json"))
    {
        return;
    }

    if (!InitializeGameplayAudio())
    {
        return;
    }


    SubscribeCollisionEvents();

    SubscribeGameplayAudioEvents();
}

void GameScene::Update(
    float deltaTime)
{
    Scene::Update(
        deltaTime
    );

    if (m_player &&
        m_player->IsAttacking())
    {
        for (auto& entity : m_entities)
        {
            if (!entity)
            {
                continue;
            }

            if (entity->IsDestroyed())
            {
                continue;
            }

            if (entity.get() ==
                m_player)
            {
                continue;
            }

            Enemy* enemy =
                dynamic_cast<Enemy*>(
                    entity.get()
                    );

            if (!enemy)
                continue;

            float dx =
                enemy->transform.position.x -
                m_player->transform.position.x;

            float dy =
                enemy->transform.position.y -
                m_player->transform.position.y;

            float distanceSquared =
                dx * dx +
                dy * dy;

            float range =
                m_player->GetAttackRange();

            if (distanceSquared <=
                range * range)
            {
                DefeatEnemy(
                    *enemy
                );
            }
        }

        m_player =
            dynamic_cast<Player*>(
                ResolveEntity(
                    m_playerHandle
                )
                );
    }
}

void GameScene::LateUpdate(
    float deltaTime)
{
    m_player =
        dynamic_cast<Player*>(
            ResolveEntity(
                m_playerHandle
            )
            );

    if (m_player)
    {
        m_camera.SetPosition(
            m_player->transform.position.x,
            m_player->transform.position.y
        );
    }

    Scene::LateUpdate(
        deltaTime
    );

    m_player =
        dynamic_cast<Player*>(
            ResolveEntity(
                m_playerHandle
            )
            );
}

void GameScene::SubmitRender(
    RenderQueue& renderQueue)
{
    if (m_tileMapRenderer)
    {
        m_tileMapRenderer->Submit(
            m_camera,
            renderQueue
        );
    }

    Scene::SubmitRender(
        renderQueue
    );
}

void GameScene::DebugRender(
    SpriteRenderer& renderer,
    DebugRenderer& debugRenderer)
{
    Scene::DebugRender(
        renderer,
        debugRenderer
    );

    if (!m_tileMap)
    {
        return;
    }

    if (!m_tileMapCollider)
    {
        return;
    }

    const float tileWidth =
        static_cast<float>(
            m_tileMap->GetTileWidth()
            );

    const float tileHeight =
        static_cast<float>(
            m_tileMap->GetTileHeight()
            );

    const auto& collisionRects =
        m_tileMapCollider->
        GetCollisionRects();

    for (const auto& rect :
        collisionRects)
    {
        const float width =
            static_cast<float>(
                rect.width
                )
            *
            tileWidth;

        const float height =
            static_cast<float>(
                rect.height
                )
            *
            tileHeight;

        const float left =
            static_cast<float>(
                rect.x
                )
            *
            tileWidth;

        const float top =
            static_cast<float>(
                rect.y
                )
            *
            tileHeight;

        const DirectX::XMFLOAT2 center
        {
            left +
            width * 0.5f,

            top +
            height * 0.5f
        };

        const DirectX::XMFLOAT2 size
        {
            width,
            height
        };

        debugRenderer.DrawRect(
            renderer,
            center,
            size,
            4.0f
        );
    }
}

void GameScene::DrawGuiContents()
{
    if (!m_tileMap ||
        !m_tileMapRenderer)
    {
        ImGui::TextDisabled(
            "TileMap is unavailable.");

        return;
    }

    EngineGui::
        DrawTileMapSettingsContents(
            *m_tileMap,
            *m_tileMapRenderer,
            m_tileMapCollider.get());
}

bool GameScene::SaveSerializedEntities(
    const std::wstring& path)
{
    SceneData sceneData;

    GameEntityFactory factory(
        *this,
        m_resources,
        m_physics,
        m_enemyAnimations
    );

    std::size_t playerCount =
        0;

    for (const auto& entity :
        m_entities)
    {
        if (!entity)
        {
            continue;
        }

        if (entity->IsDestroyed())
        {
            continue;
        }

        SerializedEntity
            serializedEntity;

        if (!factory.Serialize(
            *entity,
            serializedEntity))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Failed to serialize "
                "runtime entity.\n"
            );

            return false;
        }

        if (serializedEntity.type ==
            "Player")
        {
            ++playerCount;
        }

        sceneData.entities.emplace_back(
            std::move(
                serializedEntity
            )
        );
    }

    if (playerCount != 1)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Serialized scene must "
            "contain exactly one Player.\n"
        );

        return false;
    }

    if (!SceneSerializer::Save(
        sceneData,
        path))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to save "
            "serialized scene.\n"
        );

        return false;
    }

    return true;
}

void GameScene::CollectDebugStats(
    DebugStats& stats) const
{
    if (!m_tileMapRenderer)
    {
        return;
    }

    const TileMapRenderStats&
        tileStats =
        m_tileMapRenderer->
        GetStats();

    stats.tileRenderItems =
        static_cast<std::uint32_t>(
            tileStats.totalRenderItems
            );

    stats.visibleTiles =
        static_cast<std::uint32_t>(
            tileStats.visibleRenderItems
            );

    stats.culledTiles =
        static_cast<std::uint32_t>(
            tileStats.culledRenderItems
            );

    stats.tileRenderLayers =
        static_cast<std::uint32_t>(
            tileStats.renderLayerCount
            );

    stats.tileMapWidth =
        tileStats.mapWidth;

    stats.tileMapHeight =
        tileStats.mapHeight;

    stats.tileWidth =
        tileStats.tileWidth;

    stats.tileHeight =
        tileStats.tileHeight;

    stats.visibleTileMinX =
        tileStats.visibleMinTileX;

    stats.visibleTileMaxX =
        tileStats.visibleMaxTileX;

    stats.visibleTileMinY =
        tileStats.visibleMinTileY;

    stats.visibleTileMaxY =
        tileStats.visibleMaxTileY;

    stats.cameraLeft =
        tileStats.cameraLeft;

    stats.cameraRight =
        tileStats.cameraRight;

    stats.cameraTop =
        tileStats.cameraTop;

    stats.cameraBottom =
        tileStats.cameraBottom;

    stats.tileMapInView =
        tileStats.mapInView;

    stats.tileCandidateCells =
        static_cast<std::uint32_t>(
            tileStats.visibleCandidateCells
            );

    stats.tileGridCells =
        static_cast<std::uint32_t>(
            tileStats.totalGridCells
            );

    if (m_tileMapCollider)
    {
        stats.tileCollisionTiles =
            static_cast<std::uint32_t>(
                m_tileMapCollider->
                GetSolidTileCount()
                );

        stats.tileCollisionShapes =
            static_cast<std::uint32_t>(
                m_tileMapCollider->
                GetShapeCount()
                );

        stats.tileCollisionLayers =
            static_cast<std::uint32_t>(
                m_tileMapCollider->
                GetCollisionLayerCount()
                );
        stats.tileCollisionMergedArea =
            static_cast<std::uint32_t>(
                m_tileMapCollider->
                GetMergedTileArea()
                );
    }
}

bool GameScene::LoadEnemyAnimations()
{
    struct AnimationAsset
    {
        CharacterAnimationState state;
        FacingDirection direction;
        const wchar_t* path;
    };

    constexpr AnimationAsset assets[]
    {
        {
            CharacterAnimationState::Idle,
            FacingDirection::Down,
            L"Engine/Assets/Animations/"
            L"Qingxiao_idle_down.json"
        },
        {
            CharacterAnimationState::Idle,
            FacingDirection::Left,
            L"Engine/Assets/Animations/"
            L"Qingxiao_idle_left.json"
        },
        {
            CharacterAnimationState::Idle,
            FacingDirection::Right,
            L"Engine/Assets/Animations/"
            L"Qingxiao_idle_right.json"
        },
        {
            CharacterAnimationState::Idle,
            FacingDirection::Up,
            L"Engine/Assets/Animations/"
            L"Qingxiao_idle_up.json"
        },

        {
            CharacterAnimationState::Walk,
            FacingDirection::Down,
            L"Engine/Assets/Animations/"
            L"Qingxiao_walk_down.json"
        },
        {
            CharacterAnimationState::Walk,
            FacingDirection::Left,
            L"Engine/Assets/Animations/"
            L"Qingxiao_walk_left.json"
        },
        {
            CharacterAnimationState::Walk,
            FacingDirection::Right,
            L"Engine/Assets/Animations/"
            L"Qingxiao_walk_right.json"
        },
        {
            CharacterAnimationState::Walk,
            FacingDirection::Up,
            L"Engine/Assets/Animations/"
            L"Qingxiao_walk_up.json"
        }
    };

    for (const AnimationAsset& asset :
        assets)
    {
        AnimationClip* clip =
            m_resources.
            LoadAnimationClip(
                asset.path
            );

        if (!clip)
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Failed to load "
                "enemy animation.\n"
            );

            return false;
        }

        m_enemyAnimations.SetClip(
            asset.state,
            asset.direction,
            clip
        );
    }

    if (!m_enemyAnimations.IsValid())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Enemy animation "
            "set is incomplete.\n"
        );

        return false;
    }

    return true;
}

bool GameScene::LoadSerializedEntities(
    const std::wstring& path)
{
    ClearEntities();

    m_player = nullptr;
    m_playerHandle =
        EntityHandle{};

    auto sceneData =
        SceneSerializer::Load(
            path
        );

    if (!sceneData)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to load "
            "serialized scene.\n"
        );

        return false;
    }

    const SerializedEntity*
        playerData =
        nullptr;

    std::size_t playerCount = 0;

    GameEntityFactory factory(
        *this,
        m_resources,
        m_physics,
        m_enemyAnimations
    );

    for (const SerializedEntity& entity :
        sceneData->entities)
    {
        if (!factory.IsSupportedType(
            entity.type))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene contains "
                "an unsupported entity type.\n"
            );

            return false;
        }

        if (entity.type == "Player")
        {
            ++playerCount;

            playerData =
                &entity;
        }
    }

    if (playerCount != 1 ||
        !playerData)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene must contain "
            "exactly one Player.\n"
        );

        return false;
    }

    Entity* playerEntity =
        factory.Create(
            *playerData,
            EntityHandle{}
        );

    m_player =
        dynamic_cast<Player*>(
            playerEntity
            );

    if (!m_player)
    {
        ClearEntities();

        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to create Player.\n"
        );

        return false;
    }

    const EntityHandle
        playerHandle =
        m_player->GetHandle();

    if (!playerHandle.IsValid())
    {
        ClearEntities();

        m_player = nullptr;
        m_playerHandle =
            EntityHandle{};

        ENGINE_DEBUG_LOG(
            "[GameScene] Player has "
            "an invalid EntityHandle.\n"
        );

        return false;
    }

    m_playerHandle =
        playerHandle;

    for (const SerializedEntity& entity :
        sceneData->entities)
    {
        if (&entity == playerData)
        {
            continue;
        }

        Entity* created =
            factory.Create(
                entity,
                playerHandle
            );

        if (!created)
        {
            ClearEntities();
            m_player = nullptr;

            ENGINE_DEBUG_LOG(
                "[GameScene] Failed to create "
                "serialized entity.\n"
            );

            return false;
        }
    }

    return true;
}

bool GameScene::InitializeGameplayAudio()
{
    StopGameplayAudio();

    m_enemyDefeatSfx =
        nullptr;

    m_bgmClip =
        nullptr;


    m_enemyDefeatSfx =
        m_resources.LoadAudioClip(
            EnemyDefeatSfxPath
        );

    if (!m_enemyDefeatSfx)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to load "
            "enemy defeat SFX.\n"
        );

        return false;
    }


    m_bgmClip =
        m_resources.LoadAudioClip(
            GameplayBgmPath
        );

    if (!m_bgmClip)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to load "
            "gameplay BGM.\n"
        );

        m_enemyDefeatSfx =
            nullptr;

        return false;
    }


    m_bgmHandle =
        m_audio.PlayMusic(
            *m_bgmClip,
            GameplayBgmVolume
        );


    if (!m_bgmHandle.IsValid())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to start "
            "gameplay BGM.\n"
        );

        m_enemyDefeatSfx =
            nullptr;

        m_bgmClip =
            nullptr;

        return false;
    }


    return true;
}

void GameScene::DefeatEnemy(
    Enemy& enemy)
{
    if (enemy.IsDestroyed())
    {
        return;
    }

    const EntityHandle enemyHandle =
        enemy.GetHandle();

    const EntityHandle instigatorHandle =
        m_playerHandle;

    const float defeatWorldX =
        enemy.transform.position.x;

    const float defeatWorldY =
        enemy.transform.position.y;

    enemy.Destroy();

    if (!enemyHandle.IsValid() ||
        !instigatorHandle.IsValid())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Enemy defeat has "
            "an invalid EntityHandle.\n"
        );

        return;
    }

    m_events.Publish(
        EnemyDefeatedEvent
        {
            enemyHandle,
            instigatorHandle,
            defeatWorldX,
            defeatWorldY
        }
    );
}

void GameScene::StopGameplayAudio()
noexcept
{
    if (m_bgmHandle.IsValid())
    {
        m_audio.Stop(
            m_bgmHandle
        );

        m_bgmHandle =
            AudioPlaybackHandle{};
    }


    m_bgmClip =
        nullptr;

    m_enemyDefeatSfx =
        nullptr;
}

void GameScene::SubscribeCollisionEvents()
{
    m_collisionEnterSubscription =
        m_events.Subscribe<
        CollisionEnterEvent
        >(
            [this](
                const CollisionEnterEvent& event)
            {
                HandleCollisionEnterEvent(
                    event
                );
            }
        );

    m_collisionExitSubscription =
        m_events.Subscribe<
        CollisionExitEvent
        >(
            [this](
                const CollisionExitEvent& event)
            {
                HandleCollisionExitEvent(
                    event
                );
            }
        );
}


void GameScene::
SubscribeGameplayAudioEvents()
{
    m_enemyDefeatedAudioSubscription =
        m_events.Subscribe<
        EnemyDefeatedEvent
        >(
            [this](
                const EnemyDefeatedEvent& event)
            {
                HandleEnemyDefeatedAudio(
                    event
                );
            }
        );
}

void GameScene::HandleCollisionEnterEvent(
    const CollisionEnterEvent& event)
{
    if (!IsPlayerCollision(
        event.entityA,
        event.entityB))
    {
        return;
    }

    ENGINE_DEBUG_LOG(
        "[Player] Collision Enter\n"
    );
}

void GameScene::HandleCollisionExitEvent(
    const CollisionExitEvent& event)
{
    if (!IsPlayerCollision(
        event.entityA,
        event.entityB))
    {
        return;
    }

    ENGINE_DEBUG_LOG(
        "[Player] Collision Exit\n"
    );
}

void GameScene::
HandleEnemyDefeatedAudio(
    const EnemyDefeatedEvent& event)
{
    (void)event;


    if (!m_enemyDefeatSfx)
    {
        return;
    }


    m_audio.PlayOneShot(
        *m_enemyDefeatSfx,
        EnemyDefeatSfxVolume
    );
}

bool GameScene::IsPlayerCollision(
    EntityHandle entityA,
    EntityHandle entityB) const noexcept
{
    if (!m_playerHandle.IsValid())
    {
        return false;
    }

    return
        entityA == m_playerHandle ||
        entityB == m_playerHandle;
}