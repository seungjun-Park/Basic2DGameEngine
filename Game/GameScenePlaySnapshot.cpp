#include "GameScene.h"

#include "Enemy.h"
#include "GameEntityFactory.h"
#include "Player.h"
#include "Engine/Renderer/Camera.h"

#include "Engine/Debug/DebugLog.h"

#include "Engine/Graphics/Texture.h"

#include "Engine/Resource/ResourceManager.h"

#include "Engine/Scene/Entity.h"

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
            ENGINE_DEBUG_LOG(
                "[GameScene] Unsupported entity "
                "type in Play snapshot.\n"
            );

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
            ENGINE_DEBUG_LOG(
                "[GameScene] Entity has no "
                "snapshot sprite texture.\n"
            );

            return false;
        }

        SerializedSprite sprite;

        if (!resources.TryGetTexturePath(
            texture,
            sprite.texturePath
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Snapshot sprite "
                "texture is not ResourceManager "
                "owned.\n"
            );

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

        //
        // GameEntityFactory::Create() configures
        // gameplay animation state for Enemy.
        //
        // Re-apply the exact visual state captured
        // before Play after that initialization.
        //

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
        ENGINE_DEBUG_LOG(
            "[GameScene] A Play snapshot "
            "already exists.\n"
        );

        return false;
    }

    if (!m_tileMap ||
        m_tileMapPath.empty())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Cannot capture Play "
            "snapshot without a TileMap.\n"
        );

        return false;
    }

    SceneData snapshot;

    snapshot.version =
        SceneData::CurrentVersion;

    snapshot.tileMapPath =
        m_tileMapPath;

    snapshot.animationBindings =
        m_animationBindings;

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

        if (!CaptureEntitySnapshot(
            *entity,
            m_resources,
            serializedEntity
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Failed to capture "
                "Play snapshot entity.\n"
            );

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
        ENGINE_DEBUG_LOG(
            "[GameScene] Play snapshot must "
            "contain exactly one Player.\n"
        );

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
        ENGINE_DEBUG_LOG(
            "[GameScene] No Play snapshot "
            "exists to restore.\n"
        );

        return false;
    }

    //
    // Do not reset m_playSnapshot until restore
    // has succeeded. A failed restoration may
    // therefore be retried.
    //

    if (!RestoreEntitiesFromSnapshot(
        *m_playSnapshot
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to restore "
            "Play snapshot.\n"
        );

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
    //
    // TileMap authoring/runtime state is NOT
    // reloaded from disk here.
    //
    // Editor TileMap modifications may have been
    // applied to runtime without having been saved.
    // Play Mode cannot modify TileMap data, so the
    // existing runtime TileMap is already the
    // correct pre-Play state.
    //

    if (!m_tileMap ||
        snapshot.tileMapPath.empty() ||
        snapshot.tileMapPath !=
        m_tileMapPath)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Play snapshot TileMap "
            "identity mismatch.\n"
        );

        return false;
    }

    //
    // Preflight everything that can reasonably
    // fail before destroying current Play entities.
    //

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
            ENGINE_DEBUG_LOG(
                "[GameScene] Play snapshot "
                "contains an unsupported "
                "entity type.\n"
            );

            return false;
        }

        if (!entity.sprite)
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Play snapshot "
                "entity has no Sprite data.\n"
            );

            return false;
        }

        if (!m_resources.LoadTexture(
            entity.sprite->
            texturePath
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Play snapshot "
                "texture preflight failed.\n"
            );

            return false;
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
        ENGINE_DEBUG_LOG(
            "[GameScene] Play snapshot must "
            "contain exactly one Player.\n"
        );

        return false;
    }

    //
    // Replace simulation entities.
    //
    // This destroys Play-created physics state,
    // velocities, AI state and destroyed entities.
    //

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

        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to restore "
            "snapshot Player.\n"
        );

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

        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to restore "
            "Player snapshot visual state.\n"
        );

        return false;
    }

    const EntityHandle playerHandle =
        m_player->GetHandle();

    if (!playerHandle.IsValid())
    {
        ClearEntities();

        m_player =
            nullptr;

        ENGINE_DEBUG_LOG(
            "[GameScene] Restored Player "
            "has an invalid handle.\n"
        );

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

            ENGINE_DEBUG_LOG(
                "[GameScene] Failed to restore "
                "snapshot entity.\n"
            );

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

            ENGINE_DEBUG_LOG(
                "[GameScene] Failed to restore "
                "snapshot entity visual state.\n"
            );

            return false;
        }
    }

    m_animationBindings =
        snapshot.animationBindings;

    //
    // LateUpdate will not run in Edit Mode.
    // Restore camera immediately so the Edit
    // viewport returns with the Player.
    //

    m_camera.SetPosition(
        m_player->
        transform.position.x,
        m_player->
        transform.position.y
    );

    return true;
}