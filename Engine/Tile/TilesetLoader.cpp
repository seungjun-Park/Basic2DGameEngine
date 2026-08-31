#include "TilesetLoader.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

std::unique_ptr<Tileset>
TilesetLoader::Load(
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

    const std::string texturePath =
        json.value(
            "texture",
            ""
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

    const int columns =
        json.value(
            "columns",
            0
        );

    const int rows =
        json.value(
            "rows",
            0
        );

    if (texturePath.empty() ||
        tileWidth <= 0 ||
        tileHeight <= 0 ||
        columns <= 0 ||
        rows <= 0)
    {
        return nullptr;
    }

    std::wstring texturePathWide(
        texturePath.begin(),
        texturePath.end()
    );

    Texture* texture =
        resources.LoadTexture(
            texturePathWide
        );

    if (!texture)
    {
        return nullptr;
    }

    auto tileset =
        std::make_unique<Tileset>();

    tileset->SetTexture(
        texture
    );

    tileset->SetTileSize(
        tileWidth,
        tileHeight
    );

    tileset->SetGridSize(
        columns,
        rows
    );

    return tileset;
}