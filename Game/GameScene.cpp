#include "GameScene.h"

#include "Player.h"
#include "Enemy.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Physics/PhysicsBody.h"

//temp
#include "Engine/Scene/Entity.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Tile/Tileset.h"

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

void GameScene::Initialize()
{
    Texture* playerTexture =
        m_resources.LoadTexture(
            L"Engine/Assets/Textures/Qingxiao.png"
        );

    Texture* enemyTexture =
        m_resources.LoadTexture(
            L"Engine/Assets/Textures/2.jpg"
        );

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

    //temp

    Tileset* testTileset =
        m_tileMap->GetTileset();

    if (testTileset &&
        testTileset->GetTexture())
    {
        constexpr TileId
            testTileIds[] =
        {
            1,
            2,
            3,
            4
        };

        for (int i = 0; i < 4; ++i)
        {
            const TileId tileId =
                testTileIds[i];

            if (!testTileset->
                IsValidTileId(
                    tileId))
            {
                continue;
            }

            Entity* testSprite =
                CreateEntity<Entity>();

            testSprite->sprite.texture =
                testTileset->GetTexture();

            testSprite->sprite.uv =
                testTileset->
                GetTileUV(
                    tileId
                );

            testSprite->sprite.layer =
                RenderLayer::Foreground;

            testSprite->sprite.zIndex =
                0.0f;

            testSprite->transform.position =
            {
                100.0f +
                    static_cast<float>(
                        i
                    ) * 80.0f,

                100.0f
            };

            testSprite->transform.scale =
            {
                64.0f,
                64.0f
            };
        }
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
        0.0f,
        0.0f
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

    for (int i = 0; i < 5; ++i)
    {
        Enemy* enemy =
            CreateEntity<Enemy>(m_physics);

        enemy->sprite.texture =
            enemyTexture;

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