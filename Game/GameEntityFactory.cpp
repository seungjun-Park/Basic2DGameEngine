#include "GameEntityFactory.h"

#include "CharacterAnimation.h"
#include "Enemy.h"
#include "Player.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Animation/Animator.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Physics/PhysicsSystem.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Serialization/SceneData.h"

#include <memory>
#include <utility>

GameEntityFactory::GameEntityFactory(
    Scene& scene,
    ResourceManager& resources,
    PhysicsSystem& physics,
    const CharacterAnimationSet& enemyAnimations
)
    :
    m_scene(
        scene
    ),
    m_resources(
        resources
    ),
    m_physics(
        physics
    ),
    m_enemyAnimations(
        enemyAnimations
    )
{
}

Entity* GameEntityFactory::Create(
    const SerializedEntity& data,
    EntityHandle playerTarget
)
{
    if (data.type ==
        "Player")
    {
        Player* player =
            m_scene.CreateEntity<Player>(
                m_physics
            );

        if (!ApplySerializedState(
            *player,
            data
        ))
        {
            player->Destroy();

            return nullptr;
        }

        player->Initialize();

        if (!ApplySerializedAnimation(
            *player,
            data
        ))
        {
            player->Destroy();

            return nullptr;
        }

        return player;
    }

    if (data.type ==
        "Enemy")
    {
        if (!playerTarget.IsValid())
        {
            ENGINE_DEBUG_LOG(
                "[GameEntityFactory] "
                "Enemy requires a valid "
                "Player target.\n"
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
            data
        ))
        {
            enemy->Destroy();

            return nullptr;
        }

        if (!enemy->SetTarget(
            playerTarget
        ))
        {
            enemy->Destroy();

            return nullptr;
        }

        enemy->Initialize();

        if (!enemy->SetAnimations(
            m_enemyAnimations
        ))
        {
            enemy->Destroy();

            return nullptr;
        }

        //
        // Explicit Editor assignment is applied
        // after the gameplay animation state has
        // initialized, so Edit Mode shows exactly
        // what the Scene document authored.
        //
        if (!ApplySerializedAnimation(
            *enemy,
            data
        ))
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
    SerializedEntity& outData
) const
{
    outData =
        SerializedEntity{};

    const bool isPlayer =
        dynamic_cast<
        const Player*
        >(
            &entity
            ) != nullptr;

    const bool isEnemy =
        dynamic_cast<
        const Enemy*
        >(
            &entity
            ) != nullptr;

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

    SerializedSprite serializedSprite;

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

    if (!texture)
    {
        ENGINE_DEBUG_LOG(
            "[GameEntityFactory] "
            "Runtime entity has no sprite "
            "texture.\n"
        );

        return false;
    }

    if (!m_resources.TryGetTexturePath(
        texture,
        serializedSprite.texturePath
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameEntityFactory] "
            "Sprite texture is not owned "
            "by ResourceManager.\n"
        );

        return false;
    }

    serializedSprite.uv =
        entity.sprite.uv;

    outData.sprite =
        std::move(
            serializedSprite
        );

    outData.animationClipPath =
        entity.
        GetAssignedAnimationClipPath();

    return true;
}

bool GameEntityFactory::IsSupportedType(
    const std::string& type
) const noexcept
{
    return
        type == "Player" ||
        type == "Enemy";
}

bool GameEntityFactory::ApplySerializedState(
    Entity& entity,
    const SerializedEntity& data
)
{
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

    entity.ClearAssignedAnimationClipPath();

    return true;
}

bool GameEntityFactory::
ApplySerializedAnimation(
    Entity& entity,
    const SerializedEntity& data
)
{
    entity.ClearAssignedAnimationClipPath();

    if (data.animationClipPath.empty())
    {
        return true;
    }

    AnimationClip* clip =
        m_resources.LoadAnimationClip(
            data.animationClipPath
        );

    if (!clip ||
        !clip->IsValid())
    {
        ENGINE_DEBUG_LOG(
            "[GameEntityFactory] "
            "Failed to load assigned "
            "AnimationClip.\n"
        );

        return false;
    }

    if (!entity.animator)
    {
        entity.animator =
            std::make_unique<Animator>(
                entity.sprite
            );
    }

    if (!entity.animator->Play(
        *clip,
        true
    ))
    {
        return false;
    }

    entity.SetAssignedAnimationClipPath(
        data.animationClipPath
    );

    return true;
}