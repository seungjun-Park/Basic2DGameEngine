#include "TileMapLoader.h"

#include "TileMap.h"
#include "Tileset.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Debug/DebugLog.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    bool TryParseRenderLayer(
        const std::string& value,
        RenderLayer& result)
    {
        if (value == "Background")
        {
            result =
                RenderLayer::Background;

            return true;
        }

        if (value == "World")
        {
            result =
                RenderLayer::World;

            return true;
        }

        if (value == "Effect")
        {
            result =
                RenderLayer::Effect;

            return true;
        }

        if (value == "Foreground")
        {
            result =
                RenderLayer::Foreground;

            return true;
        }

        if (value == "UI")
        {
            result =
                RenderLayer::UI;

            return true;
        }

        if (value == "Debug")
        {
            result =
                RenderLayer::Debug;

            return true;
        }

        return false;
    }

    bool TryParseLayerType(
        const std::string& value,
        TileLayerType& result)
    {
        if (value == "Render")
        {
            result =
                TileLayerType::Render;

            return true;
        }

        if (value == "Collision")
        {
            result =
                TileLayerType::Collision;

            return true;
        }

        return false;
    }
}

std::unique_ptr<TileMap>
TileMapLoader::Load(
    const std::wstring& path,
    ResourceManager& resources)
{
    std::ifstream file
    {
        std::filesystem::path(
            path
        )
    };

    if (!file.is_open())
    {
        ENGINE_DEBUG_LOG(
            "Failed to open file."
        );

        return nullptr;
    }

    try
    {
        nlohmann::json json;

        file >> json;

        if (!json.is_object())
        {
            ENGINE_DEBUG_LOG(
                "Root must be a JSON object."
            );

            return nullptr;
        }

        const int width =
            json.value(
                "width",
                0
            );

        const int height =
            json.value(
                "height",
                0
            );

        const int tileWidth =
            json.value(
                "tileWidth",
                0
            );

        const int tileHeight =
            json.value(
                "tileHeight",
                0
            );

        const std::string tilesetPath =
            json.value(
                "tileset",
                ""
            );

        if (width <= 0 ||
            height <= 0)
        {
            ENGINE_DEBUG_LOG(
                "Invalid map size."
            );

            return nullptr;
        }

        if (tileWidth <= 0 ||
            tileHeight <= 0)
        {
            ENGINE_DEBUG_LOG(
                "Invalid tile size."
            );

            return nullptr;
        }

        if (tilesetPath.empty())
        {
            ENGINE_DEBUG_LOG(
                "Tileset path is empty."
            );

            return nullptr;
        }

        if (!json.contains("layers") ||
            !json["layers"].is_array())
        {
            ENGINE_DEBUG_LOG(
                "Missing layers array."
            );

            return nullptr;
        }

        const std::wstring
            tilesetPathWide(
                tilesetPath.begin(),
                tilesetPath.end()
            );

        Tileset* tileset =
            resources.LoadTileset(
                tilesetPathWide
            );

        if (!tileset)
        {
            ENGINE_DEBUG_LOG(
                "Failed to load tileset."
            );

            return nullptr;
        }

        const std::size_t expectedSize =
            static_cast<std::size_t>(
                width
                ) *
            static_cast<std::size_t>(
                height
                );

        auto tileMap =
            std::make_unique<TileMap>();

        tileMap->SetSize(
            width,
            height
        );

        tileMap->SetTileSize(
            tileWidth,
            tileHeight
        );

        tileMap->SetTileset(
            tileset
        );

        for (const auto& layerJson :
            json["layers"])
        {
            if (!layerJson.is_object())
            {
                ENGINE_DEBUG_LOG(
                    "Layer must be an object."
                );

                return nullptr;
            }

            TileLayer layer;

            layer.name =
                layerJson.value(
                    "name",
                    "Unnamed"
                );

            const std::string
                typeString =
                layerJson.value(
                    "type",
                    ""
                );

            if (!TryParseLayerType(
                typeString,
                layer.type))
            {
                ENGINE_DEBUG_LOG(
                    "Invalid layer type."
                );

                return nullptr;
            }

            if (layer.type ==
                TileLayerType::Render)
            {
                const std::string
                    renderLayerString =
                    layerJson.value(
                        "renderLayer",
                        ""
                    );

                if (!TryParseRenderLayer(
                    renderLayerString,
                    layer.renderLayer))
                {
                    ENGINE_DEBUG_LOG(
                        "Invalid render layer."
                    );

                    return nullptr;
                }
            }
            else
            {
                // Collision layer에서는
                // 렌더 순서가 사용되지 않는다.
                layer.renderLayer =
                    RenderLayer::World;
            }

            layer.visible =
                layerJson.value(
                    "visible",
                    true
                );

            if (!layerJson.contains(
                "tiles") ||
                !layerJson["tiles"].
                is_array())
            {
                ENGINE_DEBUG_LOG(
                    "Layer is missing tile array."
                );

                return nullptr;
            }

            layer.tiles =
                layerJson["tiles"].
                get<
                std::vector<TileId>
                >();

            if (layer.tiles.size() !=
                expectedSize)
            {
                ENGINE_DEBUG_LOG(
                    "Layer tile count does not "
                    "match map size."
                );

                return nullptr;
            }

            if (layer.type == TileLayerType::Render)
            {
                //
                // Render Layer contract:
                //
                // 0   = Empty
                // 1~N = Tileset TileId
                //
                for (TileId tileId : layer.tiles)
                {
                    if (tileId == InvalidTileId)
                    {
                        continue;
                    }

                    if (!tileset->IsValidTileId(tileId))
                    {
                        ENGINE_DEBUG_LOG(
                            "Render layer contains an invalid TileId."
                        );

                        return nullptr;
                    }
                }
            }
            else
            {
                //
                // Collision Layer contract:
                //
                // 0 = Empty
                // 1 = Solid
                //
                // Collision value는 Tileset index가 아니다.
                //
                for (TileId tileId : layer.tiles)
                {
                    if (tileId != EmptyCollisionTile &&
                        !IsSolidCollisionTile(tileId))
                    {
                        ENGINE_DEBUG_LOG(
                            "Collision layer contains an invalid value. "
                            "Only 0 (Empty) and 1 (Solid) are allowed."
                        );

                        return nullptr;
                    }
                }
            }

            tileMap->AddLayer(
                std::move(layer)
            );
        }

        return tileMap;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        ENGINE_DEBUG_LOG(
            e.what()
        );

        return nullptr;
    }
}