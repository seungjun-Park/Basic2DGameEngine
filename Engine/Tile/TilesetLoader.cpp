#include "TilesetLoader.h"

#include "Tileset.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <filesystem>
#include <fstream>

namespace
{
    void LogTilesetError(
        const char* message)
    {
        OutputDebugStringA(
            "[TilesetLoader] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
    }
}

std::unique_ptr<Tileset>
TilesetLoader::Load(
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
        LogTilesetError(
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
            LogTilesetError(
                "Root must be a JSON object."
            );

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

        if (texturePath.empty())
        {
            LogTilesetError(
                "Texture path is empty."
            );

            return nullptr;
        }

        if (tileWidth <= 0 ||
            tileHeight <= 0)
        {
            LogTilesetError(
                "Invalid tile size."
            );

            return nullptr;
        }

        if (columns <= 0 ||
            rows <= 0)
        {
            LogTilesetError(
                "Invalid grid size."
            );

            return nullptr;
        }

        const std::wstring
            texturePathWide(
                texturePath.begin(),
                texturePath.end()
            );

        Texture* texture =
            resources.LoadTexture(
                texturePathWide
            );

        if (!texture)
        {
            LogTilesetError(
                "Failed to load texture."
            );

            return nullptr;
        }

        const int expectedWidth =
            tileWidth *
            columns;

        const int expectedHeight =
            tileHeight *
            rows;

        if (texture->GetWidth() !=
            expectedWidth ||
            texture->GetHeight() !=
            expectedHeight)
        {
            LogTilesetError(
                "Texture size does not match "
                "tile grid."
            );

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
    catch (
        const nlohmann::json::exception& e)
    {
        LogTilesetError(
            e.what()
        );

        return nullptr;
    }
}