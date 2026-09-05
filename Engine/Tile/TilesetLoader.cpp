#include "TilesetLoader.h"

#include "Tileset.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TilesetSerializer.h"
#include "Engine/Tile/TilesetData.h"

#include <cstdint>
#include <limits>
#include <memory>

namespace
{
    bool CalculateRequiredExtent(
        int tileSize,
        int count,
        int margin,
        int spacing,
        std::int64_t& outExtent
    ) noexcept
    {
        outExtent = 0;

        if (tileSize <= 0 ||
            count <= 0 ||
            margin < 0 ||
            spacing < 0)
        {
            return false;
        }

        const std::int64_t tile =
            static_cast<std::int64_t>(
                tileSize
                );

        const std::int64_t itemCount =
            static_cast<std::int64_t>(
                count
                );

        const std::int64_t outerMargin =
            static_cast<std::int64_t>(
                margin
                );

        const std::int64_t gap =
            static_cast<std::int64_t>(
                spacing
                );

        const std::int64_t stride =
            tile +
            gap;

        const std::int64_t stepCount =
            itemCount -
            1;

        constexpr std::int64_t maximum =
            std::numeric_limits<
            std::int64_t
            >::max();

        if (outerMargin >
            maximum - tile)
        {
            return false;
        }

        const std::int64_t base =
            outerMargin +
            tile;

        if (stepCount > 0)
        {
            if (stride <= 0)
            {
                return false;
            }

            if (stepCount >
                (maximum - base) /
                stride)
            {
                return false;
            }
        }

        outExtent =
            base +
            stepCount *
            stride;

        return true;
    }
}

std::unique_ptr<Tileset>
TilesetLoader::Load(
    const std::wstring& path,
    ResourceManager& resources
)
{
    auto data =
        TilesetSerializer::Load(
            path
        );

    if (!data)
    {
        return nullptr;
    }

    Texture* texture =
        resources.LoadTexture(
            data->texturePath
        );

    if (!texture)
    {
        TILESET_DEBUG_LOG(
            "Failed to load Tileset texture."
        );

        return nullptr;
    }

    std::int64_t requiredWidth = 0;
    std::int64_t requiredHeight = 0;

    if (!CalculateRequiredExtent(
        data->tileWidth,
        data->columns,
        data->margin,
        data->spacing,
        requiredWidth
    ) ||
        !CalculateRequiredExtent(
            data->tileHeight,
            data->rows,
            data->margin,
            data->spacing,
            requiredHeight
        ))
    {
        TILESET_DEBUG_LOG(
            "Tileset layout extent "
            "calculation failed."
        );

        return nullptr;
    }

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
        std::make_unique<
        Tileset
        >();

    tileset->SetTexture(
        texture
    );

    tileset->SetTileSize(
        data->tileWidth,
        data->tileHeight
    );

    tileset->SetGridSize(
        data->columns,
        data->rows
    );

    tileset->SetMargin(
        data->margin
    );

    tileset->SetSpacing(
        data->spacing
    );

    return tileset;
}