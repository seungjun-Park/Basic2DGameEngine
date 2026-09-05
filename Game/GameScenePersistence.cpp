#include "GameScene.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Audio/AudioClip.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/SceneSerializer.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Tile/TileMapCollider.h"
#include "Engine/Tile/TileMapRenderer.h"

#include <array>
#include <cmath>
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

    constexpr std::string_view
        GameplayBgmSlot =
        "Gameplay.BGM";

    constexpr std::string_view
        EnemyDefeatSfxSlot =
        "Gameplay.EnemyDefeat";

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

    const SerializedAudioBinding*
        FindAudioBinding(
            const SceneData& sceneData,
            std::string_view slot
        ) noexcept
    {
        for (const SerializedAudioBinding&
            binding :
            sceneData.audioBindings)
        {
            if (binding.slot ==
                slot)
            {
                return
                    &binding;
            }
        }

        return nullptr;
    }

    bool IsValidAudioVolume(
        float volume
    ) noexcept
    {
        return
            std::isfinite(
                volume
            ) &&
            volume >= 0.0f &&
            volume <= 1.0f;
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

    {
        TileMapRenderer renderer;

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

        TileMapCollider collider;

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
    // Animation preflight
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
    // Audio preflight
    //
    // Empty audioBindings is supported for old
    // V1/V2 scenes and means "audio disabled".
    //

    const SerializedAudioBinding*
        bgmBinding =
        FindAudioBinding(
            *sceneData,
            GameplayBgmSlot
        );

    const SerializedAudioBinding*
        enemyDefeatBinding =
        FindAudioBinding(
            *sceneData,
            EnemyDefeatSfxSlot
        );

    if (bgmBinding ||
        enemyDefeatBinding)
    {
        if (!bgmBinding ||
            !enemyDefeatBinding)
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene gameplay audio "
                "bindings are incomplete.\n"
            );

            return false;
        }

        if (!IsValidAudioVolume(
            bgmBinding->volume
        ) ||
            !IsValidAudioVolume(
                enemyDefeatBinding->volume
            ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene gameplay audio "
                "volume is invalid.\n"
            );

            return false;
        }

        if (!m_resources.LoadAudioClip(
            bgmBinding->clipPath
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene BGM "
                "preflight failed.\n"
            );

            return false;
        }

        if (!m_resources.LoadAudioClip(
            enemyDefeatBinding->clipPath
        ))
        {
            ENGINE_DEBUG_LOG(
                "[GameScene] Scene enemy defeat "
                "SFX preflight failed.\n"
            );

            return false;
        }
    }

    //
    // Entity preflight
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

    if (!LoadSerializedEntities(
        path
    ))
    {
        return false;
    }

    //
    // The Scene has now supplied m_audioBindings.
    //

    if (!InitializeGameplayAudio())
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to apply "
            "loaded Scene audio bindings.\n"
        );

        return false;
    }

    return true;
}