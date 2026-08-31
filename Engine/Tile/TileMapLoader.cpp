#include "TileMapLoader.h"

#include "Tileset.h"

#include "Engine/Resource/ResourceManager.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace
{
    RenderLayer ParseRenderLayer(
        const std::string& value)
    {
        if (value == "Background")
            return RenderLayer::Background;

        if (value == "Effect")
            return RenderLayer::Effect;

        if (value == "Foreground")
            return RenderLayer::Foreground;

        if (value == "UI")
            return RenderLayer::UI;

        return RenderLayer::World;
    }

    TileLayerType ParseLayerType(
        const std::string& value)
    {
        if (value == "Collision")
        {
            return
                TileLayerType::Collision;
        }

        return
            TileLayerType::Render;
    }
}

std::unique_ptr<TileMap>
TileMapLoader::Load(
    const std::wstring& path,
    ResourceManager& resources)
{
    std::ifstream file{
        std::filesystem::path(
            path
        )
    };

    if (!file.is_open())
    {
        return nullptr;
    }

    nlohmann::json json;

    try
    {
        file >> json;
    }
    catch (...)
    {
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
        height <= 0 ||
        tileWidth <= 0 ||
        tileHeight <= 0 ||
        tilesetPath.empty())
    {
        return nullptr;
    }

    const std::wstring tilesetWide(
        tilesetPath.begin(),
        tilesetPath.end()
    );

    Tileset* tileset =
        resources.LoadTileset(
            tilesetWide
        );

    if (!tileset)
    {
        return nullptr;
    }

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

    if (!json.contains("layers") ||
        !json["layers"].is_array())
    {
        return nullptr;
    }

    for (const auto& layerJson :
        json["layers"])
    {
        TileLayer layer;

        layer.name =
            layerJson.value(
                "name",
                "Unnamed"
            );

        layer.type =
            ParseLayerType(
                layerJson.value(
                    "type",
                    "Render"
                )
            );

        layer.renderLayer =
            ParseRenderLayer(
                layerJson.value(
                    "renderLayer",
                    "World"
                )
            );

        layer.visible =
            layerJson.value(
                "visible",
                true
            );

        if (layerJson.contains("tiles") &&
            layerJson["tiles"].
            is_array())
        {
            layer.tiles =
                layerJson["tiles"].
                get<
                std::vector<TileId>
                >();
        }

        const std::size_t expectedSize =
            static_cast<std::size_t>(
                width * height
                );

        if (layer.tiles.size() <
            expectedSize)
        {
            layer.tiles.resize(
                expectedSize,
                InvalidTileId
            );
        }

        tileMap->AddLayer(
            std::move(layer)
        );
    }

    return tileMap;
}