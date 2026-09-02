#include "GameEntityFactory.h"

#include "Player.h"
#include "Enemy.h"
#include "CharacterAnimation.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Physics/PhysicsSystem.h"

#include "Engine/Serialization/SceneData.h"

#include <Windows.h>

GameEntityFactory::GameEntityFactory(
    Scene& scene,
    ResourceManager& resources,
    PhysicsSystem& physics,
    const CharacterAnimationSet& enemyAnimations)
    :
    m_scene(scene),
    m_resources(resources),
    m_physics(physics),
    m_enemyAnimations(enemyAnimations)
{
}

bool GameEntityFactory::IsSupportedType(
    const std::string& type) const noexcept
{
    return
        type == "Player" ||
        type == "Enemy";
}

bool GameEntityFactory::ApplySerializedState(
    Entity& entity,
    const SerializedEntity& data)
{
    //
    // PhysicsBody 생성 전에 Transform이
    // 완성되어 있어야 한다.
    //
    entity.transform =
        data.transform;

    entity.active =
        data.active;

    if (!data.sprite)
    {
        return false;
    }

    const SerializedSprite&
        serializedSprite =
        *data.sprite;

    Texture* texture =
        m_resources.LoadTexture(
            serializedSprite.texturePath
        );

    if (!texture)
    {
        OutputDebugStringA(
            "[GameEntityFactory] "
            "Failed to load sprite texture.\n"
        );

        return false;
    }

    entity.sprite.texture =
        texture;

    entity.sprite.visible =
        serializedSprite.visible;

    entity.sprite.layer =
        serializedSprite.layer;

    entity.sprite.zIndex =
        serializedSprite.zIndex;

    entity.sprite.useYSort =
        serializedSprite.useYSort;

    entity.sprite.blendMode =
        serializedSprite.blendMode;

    entity.sprite.uv =
        serializedSprite.uv;

    return true;
}

Entity* GameEntityFactory::Create(
    const SerializedEntity& data,
    Player* playerTarget)
{
    if (data.type == "Player")
    {
        Player* player =
            m_scene.CreateEntity<Player>(
                m_physics
            );

        if (!ApplySerializedState(
            *player,
            data))
        {
            player->Destroy();

            return nullptr;
        }

        //
        // 반드시 serialized Transform 적용 후.
        //
        player->Initialize();

        return player;
    }

    if (data.type == "Enemy")
    {
        if (!playerTarget)
        {
            OutputDebugStringA(
                "[GameEntityFactory] "
                "Enemy requires a Player target.\n"
            );

            return nullptr;
        }

        Enemy* enemy =
            m_scene.CreateEntity<Enemy>(
                m_physics
            );

        if (!ApplySerializedState(
            *enemy,
            data))
        {
            enemy->Destroy();

            return nullptr;
        }

        enemy->SetTarget(
            playerTarget
        );

        //
        // 역시 Transform 적용 후 Physics 생성.
        //
        enemy->Initialize();

        if (!enemy->SetAnimations(
            m_enemyAnimations))
        {
            enemy->Destroy();

            return nullptr;
        }

        return enemy;
    }

    OutputDebugStringA(
        "[GameEntityFactory] "
        "Unsupported entity type.\n"
    );

    return nullptr;
}