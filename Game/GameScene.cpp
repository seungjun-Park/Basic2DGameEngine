#include "GameScene.h"

#include "Player.h"
#include "Enemy.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Collision/Collision.h"

GameScene::GameScene(
    ResourceManager& resources,
    Camera& camera)
    : m_resources(resources),
    m_camera(camera)
{
}

void GameScene::Initialize()
{
    Texture* playerTexture =
        m_resources.LoadTexture(
            L"Engine/Assets/Textures/1.jpg"
        );

    Texture* enemyTexture =
        m_resources.LoadTexture(
            L"Engine/Assets/Textures/2.jpg"
        );

    if (!playerTexture)
    {
        return;
    }

    m_player =
        CreateEntity<Player>();

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

    m_player->collider.size =
    {
        64.0f,
        64.0f
    };

    m_player->Initialize();

    for (int i = 0; i < 5; ++i)
    {
        Enemy* enemy =
            CreateEntity<Enemy>();

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

        enemy->collider.size =
        {
            64.0f,
            64.0f
        };

        enemy->SetTarget(
            m_player
        );

        enemy->Initialize();
    }
}

void GameScene::Update(
    float deltaTime)
{
    Scene::Update(
        deltaTime
    );

    if (m_player)
    {
        m_camera.SetPosition(
            m_player->transform.position.x,
            m_player->transform.position.y
        );
    }

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

        if (Collision::CheckAABB(
            *m_player,
            *enemy))
        {
            OutputDebugStringA(
                "[Collision] Player hit Enemy\n"
            );
        }
    }
}