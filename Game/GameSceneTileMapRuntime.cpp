#include "GameScene.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TileMapSerializer.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Tile/TileMapCollider.h"
#include "Engine/Tile/TileMapData.h"
#include "Engine/Tile/TileMapRenderer.h"
#include "Engine/Tile/Tileset.h"
#include "Engine/Tile/TilesetLoader.h"

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace
{
    struct RuntimeLayerVisibility
    {
        std::size_t sourceLayerIndex = 0;

        bool visible = true;
    };

    bool BuildRuntimeCandidate(
        const TileMapData& data,
        Tileset& tileset,
        TileMap& outTileMap
    )
    {
        if (!TileMapSerializer::Validate(
            data
        ))
        {
            return false;
        }

        for (const TileLayer& layer :
            data.layers)
        {
            if (layer.type ==
                TileLayerType::Render)
            {
                for (TileId tileId :
                layer.tiles)
                {
                    if (tileId ==
                        InvalidTileId)
                    {
                        continue;
                    }

                    if (!tileset.IsValidTileId(
                        tileId
                    ))
                    {
                        return false;
                    }
                }
            }
            else if (
                layer.type ==
                TileLayerType::Collision)
            {
                for (TileId tileId :
                layer.tiles)
                {
                    if (tileId !=
                        EmptyCollisionTile &&
                        !IsSolidCollisionTile(
                            tileId
                        ))
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }

        outTileMap.SetSize(
            data.width,
            data.height
        );

        outTileMap.SetTileSize(
            data.tileWidth,
            data.tileHeight
        );

        outTileMap.SetTileset(
            &tileset
        );

        for (const TileLayer& layer :
            data.layers)
        {
            outTileMap.AddLayer(
                layer
            );
        }

        return true;
    }
}

bool GameScene::IsUsingTileMap(
    const std::wstring& path
) const noexcept
{
    if (!m_tileMap)
    {
        return false;
    }

    if (m_tileMapPath.empty() ||
        path.empty())
    {
        return false;
    }

    return
        path ==
        m_tileMapPath;
}

bool GameScene::ApplyTileMapData(
    const std::wstring& path,
    const TileMapData& data
)
{
    if (!IsUsingTileMap(
        path
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Runtime TileMap apply "
            "rejected: the scene is not using "
            "this TileMap.\n"
        );

        return false;
    }

    if (!TileMapSerializer::Validate(
        data
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Runtime TileMap apply "
            "rejected invalid TileMapData.\n"
        );

        return false;
    }

    //
    // Load a fresh Tileset directly through
    // TilesetLoader.
    //
    // ResourceManager::LoadTileset() may return
    // an older cached object after Tileset authoring,
    // so the candidate must be built from the
    // current serialized Tileset contents.
    //

    std::unique_ptr<Tileset>
        freshTileset =
        TilesetLoader::Load(
            data.tilesetPath,
            m_resources
        );

    if (!freshTileset)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to build fresh "
            "Tileset for runtime apply.\n"
        );

        return false;
    }

    //
    // Obtain the stable ResourceManager-owned
    // Tileset object that the final TileMap must
    // reference.
    //

    Tileset* managedTileset =
        m_resources.LoadTileset(
            data.tilesetPath
        );

    if (!managedTileset)
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to resolve "
            "managed Tileset for runtime apply.\n"
        );

        return false;
    }

    //
    // Build the entire candidate using the fresh
    // Tileset before touching current runtime state.
    //

    TileMap candidateTileMap;

    if (!BuildRuntimeCandidate(
        data,
        *freshTileset,
        candidateTileMap
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to construct "
            "runtime TileMap candidate.\n"
        );

        return false;
    }

    //
    // Preserve runtime visibility overrides.
    //
    // TileLayer::visible is serialized state.
    // TileMapRenderer visibility is runtime GUI
    // state and must not be merged into it.
    //

    std::vector<RuntimeLayerVisibility>
        runtimeVisibility;

    if (m_tileMapRenderer)
    {
        for (std::size_t layerIndex = 0;
            layerIndex <
            data.layers.size();
            ++layerIndex)
        {
            const TileLayer& layer =
                data.layers[
                    layerIndex
                ];

            if (layer.type !=
                TileLayerType::Render)
            {
                continue;
            }

            if (!m_tileMapRenderer->
                IsRenderLayerCached(
                    layerIndex
                ))
            {
                continue;
            }

            runtimeVisibility.push_back(
                RuntimeLayerVisibility
                {
                    layerIndex,
                    m_tileMapRenderer->
                        IsRenderLayerVisible(
                            layerIndex
                        )
                }
            );
        }
    }

    auto candidateRenderer =
        std::make_unique<
        TileMapRenderer
        >();

    if (!candidateRenderer->Build(
        candidateTileMap
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to rebuild "
            "TileMapRenderer.\n"
        );

        return false;
    }

    for (const RuntimeLayerVisibility&
        visibility :
        runtimeVisibility)
    {
        if (!candidateRenderer->
            IsRenderLayerCached(
                visibility.sourceLayerIndex
            ))
        {
            continue;
        }

        candidateRenderer->
            SetRenderLayerVisible(
                visibility.sourceLayerIndex,
                visibility.visible
            );
    }

    auto candidateCollider =
        std::make_unique<
        TileMapCollider
        >();

    if (!candidateCollider->Build(
        candidateTileMap,
        m_physics
    ))
    {
        ENGINE_DEBUG_LOG(
            "[GameScene] Failed to rebuild "
            "TileMapCollider.\n"
        );

        return false;
    }

    //
    // Everything that can fail has succeeded.
    //
    // From here onward only address-stable,
    // non-failing state replacement is performed.
    //

    managedTileset->ReplaceContents(
        *freshTileset
    );

    candidateTileMap.SetTileset(
        managedTileset
    );

    m_tileMap->ReplaceContents(
        std::move(
            candidateTileMap
        )
    );

    m_tileMapRenderer =
        std::move(
            candidateRenderer
        );

    m_tileMapCollider =
        std::move(
            candidateCollider
        );

    return true;
}