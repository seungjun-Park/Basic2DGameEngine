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
                RequiredAnimationSlots[index])
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
        return false;
    }

    TileMap* tileMap =
        m_resources.LoadTileMap(
            sceneData->tileMapPath
        );

    if (!tileMap)
    {
        return false;
    }

    {
        TileMapRenderer renderer;

        if (!renderer.Build(
            *tileMap
        ))
        {
            return false;
        }

        TileMapCollider collider;

        if (!collider.Build(
            *tileMap,
            m_physics
        ))
        {
            return false;
        }
    }

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
            return false;
        }

        AnimationClip* clip =
            m_resources.LoadAnimationClip(
                binding.clipPath
            );

        if (!clip ||
            !clip->IsValid())
        {
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
            return false;
        }
    }

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
            return false;
        }

        if (!IsValidAudioVolume(
            bgmBinding->volume
        ) ||
            !IsValidAudioVolume(
                enemyDefeatBinding->volume
            ))
        {
            return false;
        }

        if (!m_resources.LoadAudioClip(
            bgmBinding->clipPath
        ))
        {
            return false;
        }

        if (!m_resources.LoadAudioClip(
            enemyDefeatBinding->clipPath
        ))
        {
            return false;
        }
    }

    std::size_t playerCount =
        0;

    for (const SerializedEntity& entity :
        sceneData->entities)
    {
        if (!IsSupportedEntityType(
            entity.type
        ))
        {
            return false;
        }

        if (entity.type ==
            "Player")
        {
            ++playerCount;
        }

        if (!entity.sprite)
        {
            return false;
        }

        Texture* texture =
            m_resources.LoadTexture(
                entity.sprite->
                texturePath
            );

        if (!texture)
        {
            return false;
        }

        //
        // V4 per-entity AnimationClip preflight.
        //

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
                ENGINE_DEBUG_LOG(
                    "[GameScene] Entity "
                    "AnimationClip preflight "
                    "failed.\n"
                );

                return false;
            }
        }
    }

    if (playerCount != 1)
    {
        return false;
    }

    if (!LoadSerializedEntities(
        path
    ))
    {
        return false;
    }

    if (!InitializeGameplayAudio())
    {
        return false;
    }

    return true;
}