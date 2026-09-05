#include "GameScene.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/SceneSerializer.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Tile/TileMapCollider.h"
#include "Engine/Tile/TileMapRenderer.h"

#include <array>
#include <cstddef>
#include <limits>
#include <string_view>

namespace
{
    constexpr std::array<
        std::string_view,
        8
    >
        RequiredAnimationSlots
    {
        "Enemy.Idle.Down",
        "Enemy.Idle.Left",
        "Enemy.Idle.Right",
        "Enemy.Idle.Up",

        "Enemy.Walk.Down",
        "Enemy.Walk.Left",
        "Enemy.Walk.Right",
        "Enemy.Walk.Up"
    };

    constexpr std::size_t
        InvalidAnimationSlot =
        std::numeric_limits<
        std::size_t
        >::max();

    std::size_t FindAnimationSlot(
        const std::string& name
    ) noexcept
    {
        for (std::size_t index = 0;
            index <
            RequiredAnimationSlots.size();
            ++index)
        {
            if (name ==
                RequiredAnimationSlots[
                    index
                ])
            {
                return index;
            }
        }

        return
            InvalidAnimationSlot;
    }

    bool IsSupportedEntityType(
        const std::string& type
    ) noexcept
    {
        return
            type == "Player" ||
            type == "Enemy";
    }
}

bool GameScene::SaveSceneDocument(
    const std::wstring& path
)
{
    if (path.empty())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene save path "
            "is empty.\n"
        );

        return false;
    }

    return
        SaveSerializedEntities(
            path
        );
}

bool GameScene::LoadSceneDocument(
    const std::wstring& path
)
{
    if (path.empty())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene load path "
            "is empty.\n"
        );

        return false;
    }

    //
    // Preflight parse.
    //
    // Do not allow an obviously malformed document
    // to reach LoadSerializedEntities(), because that
    // function owns the actual runtime replacement.
    //

    auto sceneData =
        SceneSerializer::Load(
            path
        );

    if (!sceneData)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene document "
            "preflight failed.\n"
        );

        return false;
    }

    if (sceneData->tileMapPath.empty())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene document "
            "does not define a TileMap.\n"
        );

        return false;
    }

    //
    // TileMap resource preflight.
    //

    TileMap* tileMap =
        m_resources.LoadTileMap(
            sceneData->tileMapPath
        );

    if (!tileMap)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene TileMap "
            "preflight failed.\n"
        );

        return false;
    }

    //
    // Renderer / collider construction preflight.
    //
    // Keep these objects in a nested scope so the
    // temporary collider is destroyed before the
    // real Scene replacement starts.
    //

    {
        TileMapRenderer
            renderer;

        if (!renderer.Build(
            *tileMap
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] TileMapRenderer "
                "preflight failed.\n"
            );

            return false;
        }

        TileMapCollider
            collider;

        if (!collider.Build(
            *tileMap,
            m_physics
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] TileMapCollider "
                "preflight failed.\n"
            );

            return false;
        }
    }

    //
    // Animation binding preflight.
    //

    std::array<
        bool,
        RequiredAnimationSlots.size()
    >
        animationSlots{};

    for (const SerializedAnimationBinding& binding :
        sceneData->animationBindings)
    {
        const std::size_t slotIndex =
            FindAnimationSlot(
                binding.slot
            );

        //
        // Match GameScene::LoadEnemyAnimations():
        // unknown extension slots are ignored.
        //

        if (slotIndex ==
            InvalidAnimationSlot)
        {
            continue;
        }

        if (animationSlots[
            slotIndex
        ])
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Duplicate required "
                "animation binding.\n"
            );

            return false;
        }

        AnimationClip* clip =
            m_resources.LoadAnimationClip(
                binding.clipPath
            );

        if (!clip ||
            !clip->IsValid())
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene animation "
                "preflight failed.\n"
            );

            return false;
        }

        animationSlots[
            slotIndex
        ] = true;
    }

    for (bool bound :
    animationSlots)
    {
        if (!bound)
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene animation "
                "bindings are incomplete.\n"
            );

            return false;
        }
    }

    //
    // Entity preflight.
    //

    std::size_t playerCount =
        0;

    for (const SerializedEntity& entity :
        sceneData->entities)
    {
        if (!IsSupportedEntityType(
            entity.type
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene contains "
                "an unsupported entity type.\n"
            );

            return false;
        }

        if (entity.type ==
            "Player")
        {
            ++playerCount;
        }

        if (!entity.sprite)
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Serialized entity "
                "has no Sprite data.\n"
            );

            return false;
        }

        Texture* texture =
            m_resources.LoadTexture(
                entity.sprite->
                texturePath
            );

        if (!texture)
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Entity texture "
                "preflight failed.\n"
            );

            return false;
        }
    }

    if (playerCount != 1)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Scene must contain "
            "exactly one Player.\n"
        );

        return false;
    }

    //
    // All non-destructive validation succeeded.
    //
    // Existing runtime replacement remains owned by
    // the already validated LoadSerializedEntities().
    //

    return
        LoadSerializedEntities(
            path
        );
}