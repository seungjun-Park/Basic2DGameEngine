#include "GameScene.h"

#include "Enemy.h"
#include "GameEntityFactory.h"
#include "Player.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Animation/AnimationClip.h"

#include <cstddef>
#include <utility>

namespace
{
    bool CaptureEntitySnapshot(
        const Entity& entity,
        ResourceManager& resources,
        SerializedEntity& outData
    )
    {
        outData =
            SerializedEntity{};

        if (dynamic_cast<
            const Player*
        >(
            &entity
            ))
        {
            outData.type =
                "Player";
        }
        else if (
            dynamic_cast<
            const Enemy*
            >(
                &entity
                ))
        {
            outData.type =
                "Enemy";
        }
        else
        {
            return false;
        }

        outData.active =
            entity.active;

        outData.transform =
            entity.transform;

        Texture* texture =
            entity.sprite.texture;

        if (!texture)
        {
            return false;
        }

        SerializedSprite sprite;

        if (!resources.TryGetTexturePath(
            texture,
            sprite.texturePath
        ))
        {
            return false;
        }

        sprite.visible =
            entity.sprite.visible;

        sprite.layer =
            entity.sprite.layer;

        sprite.zIndex =
            entity.sprite.zIndex;

        sprite.useYSort =
            entity.sprite.useYSort;

        sprite.blendMode =
            entity.sprite.blendMode;

        sprite.uv =
            entity.sprite.uv;

        outData.sprite =
            std::move(
                sprite
            );

        outData.animationClipPath =
            entity.
            GetAssignedAnimationClipPath();

        return true;
    }

    bool RestoreSnapshotSprite(
        Entity& entity,
        const SerializedEntity& data,
        ResourceManager& resources
    )
    {
        if (!data.sprite)
        {
            return false;
        }

        const SerializedSprite& sprite =
            *data.sprite;

        Texture* texture =
            resources.LoadTexture(
                sprite.texturePath
            );

        if (!texture)
        {
            return false;
        }

        entity.sprite.texture =
            texture;

        entity.sprite.visible =
            sprite.visible;

        entity.sprite.layer =
            sprite.layer;

        entity.sprite.zIndex =
            sprite.zIndex;

        entity.sprite.useYSort =
            sprite.useYSort;

        entity.sprite.blendMode =
            sprite.blendMode;

        entity.sprite.uv =
            sprite.uv;

        return true;
    }
}

bool GameScene::CapturePlaySnapshot()
{
    if (m_playSnapshot.has_value())
    {
        return false;
    }

    if (!m_tileMap ||
        m_tileMapPath.empty())
    {
        return false;
    }

    SceneData snapshot;

    snapshot.version =
        SceneData::CurrentVersion;

    snapshot.tileMapPath =
        m_tileMapPath;

    snapshot.animationBindings =
        m_animationBindings;

    snapshot.audioBindings =
        m_audioBindings;

    std::size_t playerCount =
        0;

    for (const auto& entity :
        m_entities)
    {
        if (!entity ||
            entity->IsDestroyed())
        {
            continue;
        }

        SerializedEntity
            serializedEntity;

        if (!CaptureEntitySnapshot(
            *entity,
            m_resources,
            serializedEntity
        ))
        {
            return false;
        }

        if (serializedEntity.type ==
            "Player")
        {
            ++playerCount;
        }

        snapshot.entities.emplace_back(
            std::move(
                serializedEntity
            )
        );
    }

    if (playerCount != 1)
    {
        return false;
    }

    m_playSnapshot =
        std::move(
            snapshot
        );

    return true;
}

bool GameScene::RestorePlaySnapshot()
{
    if (!m_playSnapshot)
    {
        return false;
    }

    if (!RestoreEntitiesFromSnapshot(
        *m_playSnapshot
    ))
    {
        return false;
    }

    m_playSnapshot.reset();

    return true;
}

void GameScene::DiscardPlaySnapshot()
noexcept
{
    m_playSnapshot.reset();
}

bool GameScene::HasPlaySnapshot()
const noexcept
{
    return
        m_playSnapshot.has_value();
}

bool GameScene::RestoreEntitiesFromSnapshot(
    const SceneData& snapshot
)
{
    if (!m_tileMap ||
        snapshot.tileMapPath.empty() ||
        snapshot.tileMapPath !=
        m_tileMapPath)
    {
        return false;
    }

    const SerializedEntity*
        playerData =
        nullptr;

    std::size_t playerCount =
        0;

    GameEntityFactory factory(
        *this,
        m_resources,
        m_physics,
        m_enemyAnimations
    );

    for (const SerializedEntity& entity :
        snapshot.entities)
    {
        if (!factory.IsSupportedType(
            entity.type
        ))
        {
            return false;
        }

        if (!entity.sprite)
        {
            return false;
        }

        if (!m_resources.LoadTexture(
            entity.sprite->
            texturePath
        ))
        {
            return false;
        }

        if (!entity.animationClipPath.empty())
        {
            AnimationClip* clip =
                m_resources.
                LoadAnimationClip(
                    entity.
                    animationClipPath
                );

            if (!clip ||
                !clip->IsValid())
            {
                return false;
            }
        }

        if (entity.type ==
            "Player")
        {
            ++playerCount;

            playerData =
                &entity;
        }
    }

    if (playerCount != 1 ||
        !playerData)
    {
        return false;
    }

    ClearEntities();

    m_player =
        nullptr;

    m_playerHandle =
        EntityHandle{};

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

        return false;
    }

    if (!RestoreSnapshotSprite(
        *m_player,
        *playerData,
        m_resources
    ))
    {
        ClearEntities();

        m_player =
            nullptr;

        return false;
    }

    const EntityHandle playerHandle =
        m_player->GetHandle();

    if (!playerHandle.IsValid())
    {
        ClearEntities();

        m_player =
            nullptr;

        return false;
    }

    m_playerHandle =
        playerHandle;

    for (const SerializedEntity& entity :
        snapshot.entities)
    {
        if (&entity ==
            playerData)
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

            m_player =
                nullptr;

            m_playerHandle =
                EntityHandle{};

            return false;
        }

        if (!RestoreSnapshotSprite(
            *created,
            entity,
            m_resources
        ))
        {
            ClearEntities();

            m_player =
                nullptr;

            m_playerHandle =
                EntityHandle{};

            return false;
        }
    }

    m_animationBindings =
        snapshot.animationBindings;

    m_audioBindings =
        snapshot.audioBindings;

    m_camera.SetPosition(
        m_player->
        transform.position.x,
        m_player->
        transform.position.y
    );

    return true;
}