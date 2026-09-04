#include "TilesetLoader.h"

#include "Tileset.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Debug/DebugLog.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <cstdint>
#include <filesystem>
#include <fstream>


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
        TILESET_DEBUG_LOG(
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
            TILESET_DEBUG_LOG(
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
            TILESET_DEBUG_LOG(
                "Texture path is empty."
            );

            return nullptr;
        }

        if (tileWidth <= 0 ||
            tileHeight <= 0)
        {
            TILESET_DEBUG_LOG(
                "Invalid tile size."
            );

            return nullptr;
        }

        if (columns <= 0 ||
            rows <= 0)
        {
            TILESET_DEBUG_LOG(
                "Invalid grid size."
            );

            return nullptr;
        }

        if (margin < 0)
        {
            TILESET_DEBUG_LOG(
                "Margin cannot be negative."
            );

            return nullptr;
        }

        if (spacing < 0)
        {
            TILESET_DEBUG_LOG(
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
            TILESET_DEBUG_LOG(
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
            TILESET_DEBUG_LOG(
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
        TILESET_DEBUG_LOG(
            e.what()
        );

        return nullptr;
    }
}