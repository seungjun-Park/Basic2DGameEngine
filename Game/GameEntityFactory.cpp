#include "GameEntityFactory.h"
#include "Engine/Debug/DebugLog.h"

#include "Player.h"
#include "Enemy.h"
#include "CharacterAnimation.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Physics/PhysicsSystem.h"

#include "Engine/Serialization/SceneData.h"
#include "Engine/Animation/AnimationClip.h"

#include <utility>

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
        ENGINE_DEBUG_LOG(
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
    EntityHandle playerTarget)
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
        if (!playerTarget.IsValid())
        {
            ENGINE_DEBUG_LOG(
                "[GameEntityFactory] "
                "Enemy requires a valid Player target.\n"
            );

            return nullptr;
        }

        Enemy* enemy =
            m_scene.CreateEntity<Enemy>(
                m_scene,
                m_physics
            );

        if (!ApplySerializedState(
            *enemy,
            data))
        {
            enemy->Destroy();

            return nullptr;
        }

        if (!enemy->SetTarget(
            playerTarget))
        {
            enemy->Destroy();

            ENGINE_DEBUG_LOG(
                "[GameEntityFactory] "
                "Enemy Player target is invalid.\n"
            );

            return nullptr;
        }

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

    ENGINE_DEBUG_LOG(
        "[GameEntityFactory] "
        "Unsupported entity type.\n"
    );

    return nullptr;
}

bool GameEntityFactory::Serialize(
    const Entity& entity,
    SerializedEntity& outData) const
{
    outData = {};

    const bool isPlayer =
        dynamic_cast<
        const Player*
        >(&entity) != nullptr;

    const bool isEnemy =
        dynamic_cast<
        const Enemy*
        >(&entity) != nullptr;

    if (isPlayer)
    {
        outData.type =
            "Player";
    }
    else if (isEnemy)
    {
        outData.type =
            "Enemy";
    }
    else
    {
        ENGINE_DEBUG_LOG(
            "[GameEntityFactory] "
            "Unsupported runtime entity type "
            "for serialization.\n"
        );

        return false;
    }

    outData.active =
        entity.active;

    outData.transform =
        entity.transform;

    SerializedSprite
        serializedSprite;

    serializedSprite.visible =
        entity.sprite.visible;

    serializedSprite.layer =
        entity.sprite.layer;

    serializedSprite.zIndex =
        entity.sprite.zIndex;

    serializedSprite.useYSort =
        entity.sprite.useYSort;

    serializedSprite.blendMode =
        entity.sprite.blendMode;

    Texture* texture =
        entity.sprite.texture;

    UVRect uv =
        entity.sprite.uv;

    //
    // Enemy의 현재 Sprite UV는 Animator에 의해
    // 계속 변경되는 transient runtime state이다.
    //
    // Scene asset에는 animation playback frame을
    // 저장하지 않는다.
    //
    // 따라서 Enemy는 canonical initial pose인
    // Idle / Down 첫 frame으로 저장한다.
    //
    if (isEnemy)
    {
        AnimationClip* idleDown =
            m_enemyAnimations.GetClip(
                CharacterAnimationState::Idle,
                FacingDirection::Down
            );

        if (!idleDown ||
            !idleDown->IsValid())
        {
            ENGINE_DEBUG_LOG(
                "[GameEntityFactory] "
                "Enemy idle animation is invalid.\n"
            );

            return false;
        }

        const AnimationFrame* frame =
            idleDown->GetFrame(
                0
            );

        if (!frame)
        {
            return false;
        }

        texture =
            idleDown->GetTexture();

        uv =
            frame->uv;
    }

    if (!texture)
    {
        ENGINE_DEBUG_LOG(
            "[GameEntityFactory] "
            "Runtime entity has no sprite texture.\n"
        );

        return false;
    }

    if (!m_resources.TryGetTexturePath(
        texture,
        serializedSprite.texturePath))
    {
        ENGINE_DEBUG_LOG(
            "[GameEntityFactory] "
            "Sprite texture is not owned "
            "by ResourceManager.\n"
        );

        return false;
    }

    serializedSprite.uv =
        uv;

    outData.sprite =
        std::move(
            serializedSprite
        );

    return true;
}