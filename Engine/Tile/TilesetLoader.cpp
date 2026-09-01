#include "TilesetLoader.h"

#include "Tileset.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <cstdint>
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

        //
        // Stage 8:
        //
        // 기존 asset과의 호환성을 위해
        // optional + default 0.
        //

        const int margin =
            json.value(
                "margin",
                0
            );

        const int spacing =
            json.value(
                "spacing",
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

        if (margin < 0)
        {
            LogTilesetError(
                "Margin cannot be negative."
            );

            return nullptr;
        }

        if (spacing < 0)
        {
            LogTilesetError(
                "Spacing cannot be negative."
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

        //
        // --------------------------------------------------
        // Stage 8 Layout Validation
        // --------------------------------------------------
        //
        // 더 이상:
        //
        // textureWidth ==
        //     tileWidth * columns
        //
        // 을 요구하지 않는다.
        //
        // 마지막 tile이 texture 내부에만
        // 들어오면 유효하다.
        //

        const std::int64_t
            strideX =
            static_cast<std::int64_t>(
                tileWidth
                )
            +
            static_cast<std::int64_t>(
                spacing
                );

        const std::int64_t
            strideY =
            static_cast<std::int64_t>(
                tileHeight
                )
            +
            static_cast<std::int64_t>(
                spacing
                );

        const std::int64_t
            requiredWidth =
            static_cast<std::int64_t>(
                margin
                )
            +
            static_cast<std::int64_t>(
                columns - 1
                )
            *
            strideX
            +
            static_cast<std::int64_t>(
                tileWidth
                );

        const std::int64_t
            requiredHeight =
            static_cast<std::int64_t>(
                margin
                )
            +
            static_cast<std::int64_t>(
                rows - 1
                )
            *
            strideY
            +
            static_cast<std::int64_t>(
                tileHeight
                );

        if (requiredWidth >
            static_cast<std::int64_t>(
                texture->GetWidth()
                ) ||
            requiredHeight >
            static_cast<std::int64_t>(
                texture->GetHeight()
                ))
        {
            LogTilesetError(
                "Tileset layout exceeds "
                "texture bounds."
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

        tileset->SetMargin(
            margin
        );

        tileset->SetSpacing(
            spacing
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