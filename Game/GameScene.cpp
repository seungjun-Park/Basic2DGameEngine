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

GameScene::GameScene(
    ResourceManager& resources,
    Camera& camera,
    PhysicsSystem& physics)
    :
    m_resources(resources),
    m_camera(camera),
    m_physics(physics)
{
}

GameScene::~GameScene() = default;

void GameScene::Initialize()
{
    Texture* playerTexture =
        m_resources.LoadTexture(
            L"Engine/Assets/Textures/Aemeath.png"
        );

    /*Texture* enemyTexture =
        m_resources.LoadTexture(
            L"Engine/Assets/Textures/Qingxiao.png"
        );*/

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

    if (!playerTexture)
    {
        return;
    }

    m_player =
        CreateEntity<Player>(m_physics);

    m_player->sprite.texture =
        playerTexture;

    m_player->transform.position =
    {
        static_cast<float>(
            m_tileMap->GetTileWidth() * 2
        ),

        static_cast<float>(
            m_tileMap->GetTileHeight() * 2
        )
    };

    m_player->transform.scale =
    {
        96.0f,
        96.0f
    };

    m_player->sprite.layer =
        RenderLayer::World;
    
    m_player->sprite.zIndex =
        10.0f;

    m_player->sprite.useYSort = true;

    m_player->Initialize();
    
    m_enemyWalkClip =
        m_resources.LoadAnimationClip(
            L"Engine/Assets/Animations/"
            L"Qingxiao_walk_front.json"
        );

    if (!m_enemyWalkClip)
    {
        OutputDebugStringA(
            "[GameScene] Failed to load "
            "enemy walk animation.\n"
        );

        return;
    }

    for (int i = 0; i < 5; ++i)
    {
        Enemy* enemy =
            CreateEntity<Enemy>(m_physics);

        /*enemy->sprite.texture =
            enemyTexture;*/

        enemy->transform.position =
        {
            300.0f + i * 150.0f,
            200.0f
        };

        enemy->transform.scale =
        {
            80.0f,
            80.0f
        };

        enemy->SetTarget(
            m_player
        );

        enemy->sprite.layer =
            RenderLayer::World;

        enemy->sprite.zIndex =
            5.0f;

        enemy->sprite.useYSort = true;

        enemy->Initialize();

        enemy->animator =
            std::make_unique<Animator>(
                enemy->sprite
            );

        if (!enemy->animator->Play(
            *m_enemyWalkClip))
        {
            OutputDebugStringA(
                "[GameScene] Failed to start "
                "enemy walk animation.\n"
            );

            enemy->animator.reset();

            return;
        }
    }
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
                enemy->Destroy();
            }
        }
    }

    for (auto& entity : m_entities)
    {
        Enemy* enemy =
            dynamic_cast<Enemy*>(
                entity.get()
                );

        if (!enemy)
            continue;
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