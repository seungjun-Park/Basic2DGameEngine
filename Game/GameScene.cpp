#include "GameScene.h"

#include "Player.h"
#include "Enemy.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Physics/PhysicsUnits.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Tile/TileMapRenderer.h"
#include "Engine/Tile/TileMapCollider.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Debug/DebugRenderer.h"
#include "Engine/Animation/AnimationClip.h"
#include "Engine/Animation/Animator.h"

#include "GameEntityFactory.h"
#include "Engine/Serialization/SceneData.h"
#include "Engine/Serialization/SceneSerializer.h"
#include "Engine/Platform/Windows/WinInput.h"
#include "Engine/Event/EventBus.h"
#include "Engine/Physics/CollisionEvents.h"
#include "GameplayEvents.h"

#include <optional>
#include <utility>
#include <cassert>


GameScene::GameScene(
    ResourceManager& resources,
    Camera& camera,
    PhysicsSystem& physics,
    EventBus& events)
    :
    m_resources(resources),
    m_camera(camera),
    m_physics(physics),
    m_events(events)
{
}

GameScene::~GameScene() = default;

void GameScene::Initialize()
{
    m_tileMap =
        m_resources.LoadTileMap(
            L"Engine/Assets/Maps/testmap.json"
        );


    if (!m_tileMap)
    {
        OutputDebugStringA(
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
        OutputDebugStringA(
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
        OutputDebugStringA(
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

    SubscribeCollisionEvents();
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
    }
}

void GameScene::LateUpdate(
    float deltaTime)
{
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
}

void GameScene::SubmitRender(
    RenderQueue& renderQueue)
{
    //
    // TileMap render data를 먼저 제출한다.
    //
    // 실제 그리기 순서는
    // submission 순서가 아니라
    // RenderQueue::Sort()가 결정한다.
    //

    if (m_tileMapRenderer)
    {
        m_tileMapRenderer->Submit(
            m_camera,
            renderQueue
        );
    }

    //
    // Player / Enemy 등 Entity 제출
    //

    Scene::SubmitRender(
        renderQueue
    );
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

void GameScene::DebugRender(
    SpriteRenderer& renderer,
    DebugRenderer& debugRenderer)
{
    //
    // 먼저 일반 Entity PhysicsBody.
    //

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

        //
        // Tile collider는 Entity collider보다
        // 조금 두껍게 표시.
        //

        debugRenderer.DrawRect(
            renderer,
            center,
            size,
            4.0f
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
            OutputDebugStringA(
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
        OutputDebugStringA(
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
    m_playerHandle =
        EntityHandle{};

    auto sceneData =
        SceneSerializer::Load(
            path
        );

    if (!sceneData)
    {
        OutputDebugStringA(
            "[GameScene] Failed to load "
            "serialized scene.\n"
        );

        return false;
    }

    const SerializedEntity*
        playerData =
        nullptr;

    std::size_t playerCount = 0;

    //
    // Preflight.
    //
    // 실제 Entity를 만들기 전에 type contract와
    // Player cardinality를 먼저 검사한다.
    //
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
            OutputDebugStringA(
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

    //
    // 현재 GameScene contract:
    //
    // 정확히 한 명의 Player가 존재해야 한다.
    //
    if (playerCount != 1 ||
        !playerData)
    {
        OutputDebugStringA(
            "[GameScene] Scene must contain "
            "exactly one Player.\n"
        );

        return false;
    }

    //
    // Pass 1:
    // JSON 배열 순서와 관계없이 Player부터 생성.
    //
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

        OutputDebugStringA(
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

        OutputDebugStringA(
            "[GameScene] Player has "
            "an invalid EntityHandle.\n"
        );

        return false;
    }

    m_playerHandle =
        playerHandle;

    //
    // Pass 2:
    // 나머지 Entity 생성.
    //
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
            //
            // Initialize 단계에서만 호출되는 loader이므로
            // partial scene을 허용하지 않는다.
            //
            ClearEntities();
            m_player = nullptr;

            OutputDebugStringA(
                "[GameScene] Failed to create "
                "serialized entity.\n"
            );

            return false;
        }
    }

    return true;
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

        //
        // deferred destruction 대기 중인 Entity는
        // persistent scene에 저장하지 않는다.
        //
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
            OutputDebugStringA(
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

    //
    // Load contract와 동일:
    // valid GameScene은 정확히 한 Player.
    //
    if (playerCount != 1)
    {
        OutputDebugStringA(
            "[GameScene] Serialized scene must "
            "contain exactly one Player.\n"
        );

        return false;
    }

    if (!SceneSerializer::Save(
        sceneData,
        path))
    {
        OutputDebugStringA(
            "[GameScene] Failed to save "
            "serialized scene.\n"
        );

        return false;
    }

    return true;
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

void GameScene::HandleCollisionEnterEvent(
    const CollisionEnterEvent& event)
{
    if (!IsPlayerCollision(
        event.entityA,
        event.entityB))
    {
        return;
    }

    OutputDebugStringA(
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

    OutputDebugStringA(
        "[Player] Collision Exit\n"
    );
}

void GameScene::DefeatEnemy(
    Enemy& enemy)
{
    //
    // Synchronous EventBus subscriber가 같은 frame 안에서
    // 다른 Enemy를 먼저 Destroy할 수도 있으므로
    // 이미 logically-dead Entity는 다시 처리하지 않는다.
    //
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

    //
    // 기존 gameplay result가 먼저 확정된다.
    //
    enemy.Destroy();

    //
    // 정상 runtime Entity라면 두 handle 모두 valid여야 한다.
    //
    // 하지만 event publication 실패 때문에 기존 gameplay
    // result까지 되돌리면 안 되므로 Destroy는 위에서 이미
    // 수행했다.
    //
    if (!enemyHandle.IsValid() ||
        !instigatorHandle.IsValid())
    {
        OutputDebugStringA(
            "[GameScene] Enemy defeat has "
            "an invalid EntityHandle.\n"
        );

        return;
    }

    //
    // EventBus는 이미 발생한 gameplay fact만 알린다.
    //
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