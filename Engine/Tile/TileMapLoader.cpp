#include "TileMapLoader.h"

#include "TileMap.h"
#include "Tileset.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TileMapSerializer.h"

#include <memory>
#include <utility>

std::unique_ptr<TileMap>
TileMapLoader::Load(
    const std::wstring& path,
    ResourceManager& resources
)
{
    auto data =
        TileMapSerializer::Load(
            path
        );

    if (!data)
    {
        return nullptr;
    }

    Tileset* tileset =
        resources.LoadTileset(
            data->tilesetPath
        );

    if (!tileset)
    {
        ENGINE_DEBUG_LOG(
            "Failed to load TileMap Tileset."
        );

        return nullptr;
    }

    for (const TileLayer& layer :
        data->layers)
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

                if (!tileset->
                    IsValidTileId(
                        tileId
                    ))
                {
                    ENGINE_DEBUG_LOG(
                        "Render layer contains "
                        "an invalid TileId."
                    );

                    return nullptr;
                }
            }
        }
        else
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
                    ENGINE_DEBUG_LOG(
                        "Collision layer contains "
                        "an invalid value."
                    );

                    return nullptr;
                }
            }
        }
    }

    auto tileMap =
        std::make_unique<
        TileMap
        >();

    tileMap->SetSize(
        data->width,
        data->height
    );

    tileMap->SetTileSize(
        data->tileWidth,
        data->tileHeight
    );

    tileMap->SetTileset(
        tileset
    );

    for (TileLayer& layer :
        data->layers)
    {
        tileMap->AddLayer(
            std::move(
                layer
            )
        );
    }

    return tileMap;
}