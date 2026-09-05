#include "TileMapSerializer.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Tile/TileMapData.h"

#include <Windows.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace
{
    std::wstring Utf8ToWide(
        const std::string& value
    )
    {
        if (value.empty())
        {
            return {};
        }

        if (value.size() >
            static_cast<std::size_t>(
                std::numeric_limits<int>::max()
                ))
        {
            return {};
        }

        const int sourceLength =
            static_cast<int>(
                value.size()
                );

        const int requiredSize =
            ::MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                sourceLength,
                nullptr,
                0
            );

        if (requiredSize <= 0)
        {
            return {};
        }

        std::wstring result(
            static_cast<std::size_t>(
                requiredSize
                ),
            L'\0'
        );

        const int convertedSize =
            ::MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                sourceLength,
                result.data(),
                requiredSize
            );

        if (convertedSize !=
            requiredSize)
        {
            return {};
        }

        return result;
    }

    std::string WideToUtf8(
        const std::wstring& value
    )
    {
        if (value.empty())
        {
            return {};
        }

        if (value.size() >
            static_cast<std::size_t>(
                std::numeric_limits<int>::max()
                ))
        {
            return {};
        }

        const int sourceLength =
            static_cast<int>(
                value.size()
                );

        const int requiredSize =
            ::WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                value.data(),
                sourceLength,
                nullptr,
                0,
                nullptr,
                nullptr
            );

        if (requiredSize <= 0)
        {
            return {};
        }

        std::string result(
            static_cast<std::size_t>(
                requiredSize
                ),
            '\0'
        );

        const int convertedSize =
            ::WideCharToMultiByte(
                CP_UTF8,
                WC_ERR_INVALID_CHARS,
                value.data(),
                sourceLength,
                result.data(),
                requiredSize,
                nullptr,
                nullptr
            );

        if (convertedSize !=
            requiredSize)
        {
            return {};
        }

        return result;
    }

    bool TryCalculateTileCount(
        int width,
        int height,
        std::size_t& result
    ) noexcept
    {
        result = 0;

        if (width <= 0 ||
            height <= 0)
        {
            return false;
        }

        const std::size_t widthValue =
            static_cast<std::size_t>(
                width
                );

        const std::size_t heightValue =
            static_cast<std::size_t>(
                height
                );

        if (widthValue >
            std::numeric_limits<
            std::size_t
            >::max() /
            heightValue)
        {
            return false;
        }

        result =
            widthValue *
            heightValue;

        return true;
    }

    bool TryParseLayerType(
        const std::string& value,
        TileLayerType& result
    ) noexcept
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

    const char* GetLayerTypeName(
        TileLayerType type
    ) noexcept
    {
        switch (type)
        {
        case TileLayerType::Render:
            return "Render";

        case TileLayerType::Collision:
            return "Collision";

        default:
            return nullptr;
        }
    }

    bool TryParseRenderLayer(
        const std::string& value,
        RenderLayer& result
    ) noexcept
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

    const char* GetRenderLayerName(
        RenderLayer layer
    ) noexcept
    {
        switch (layer)
        {
        case RenderLayer::Background:
            return "Background";

        case RenderLayer::World:
            return "World";

        case RenderLayer::Effect:
            return "Effect";

        case RenderLayer::Foreground:
            return "Foreground";

        case RenderLayer::UI:
            return "UI";

        case RenderLayer::Debug:
            return "Debug";

        default:
            return nullptr;
        }
    }
}

bool TileMapSerializer::Save(
    const TileMapData& data,
    const std::wstring& path
)
{
    if (!Validate(
        data
    ))
    {
        ENGINE_DEBUG_LOG(
            "Cannot save invalid TileMapData."
        );

        return false;
    }

    if (path.empty())
    {
        ENGINE_DEBUG_LOG(
            "TileMap output path is empty."
        );

        return false;
    }

    const std::string tilesetPath =
        WideToUtf8(
            data.tilesetPath
        );

    if (tilesetPath.empty())
    {
        ENGINE_DEBUG_LOG(
            "Failed to encode TileMap "
            "Tileset path."
        );

        return false;
    }

    try
    {
        nlohmann::json root;

        root["width"] =
            data.width;

        root["height"] =
            data.height;

        root["tileWidth"] =
            data.tileWidth;

        root["tileHeight"] =
            data.tileHeight;

        root["tileset"] =
            tilesetPath;

        root["layers"] =
            nlohmann::json::array();

        for (const TileLayer& layer :
            data.layers)
        {
            const char* typeName =
                GetLayerTypeName(
                    layer.type
                );

            if (!typeName)
            {
                return false;
            }

            nlohmann::json layerJson;

            layerJson["name"] =
                layer.name;

            layerJson["type"] =
                typeName;

            if (layer.type ==
                TileLayerType::Render)
            {
                const char* renderLayerName =
                    GetRenderLayerName(
                        layer.renderLayer
                    );

                if (!renderLayerName)
                {
                    return false;
                }

                layerJson["renderLayer"] =
                    renderLayerName;
            }

            layerJson["visible"] =
                layer.visible;

            layerJson["tiles"] =
                layer.tiles;

            root["layers"].
                push_back(
                    std::move(
                        layerJson
                    )
                );
        }

        std::ofstream file
        {
            std::filesystem::path(
                path
            )
        };

        if (!file.is_open())
        {
            ENGINE_DEBUG_LOG(
                "Failed to open TileMap "
                "output file."
            );

            return false;
        }

        file <<
            root.dump(4);

        if (!file.good())
        {
            ENGINE_DEBUG_LOG(
                "Failed to write TileMap."
            );

            return false;
        }

        return true;
    }
    catch (
        const nlohmann::json::exception& e
        )
    {
        ENGINE_DEBUG_LOG(
            e.what()
        );

        return false;
    }
}

std::unique_ptr<TileMapData>
TileMapSerializer::Load(
    const std::wstring& path
)
{
    if (path.empty())
    {
        ENGINE_DEBUG_LOG(
            "TileMap path is empty."
        );

        return nullptr;
    }

    std::ifstream file
    {
        std::filesystem::path(
            path
        )
    };

    if (!file.is_open())
    {
        ENGINE_DEBUG_LOG(
            "Failed to open TileMap."
        );

        return nullptr;
    }

    try
    {
        nlohmann::json root;

        file >> root;

        if (!root.is_object())
        {
            ENGINE_DEBUG_LOG(
                "TileMap root must be "
                "an object."
            );

            return nullptr;
        }

        const int width =
            root.value(
                "width",
                0
            );

        const int height =
            root.value(
                "height",
                0
            );

        const int tileWidth =
            root.value(
                "tileWidth",
                0
            );

        const int tileHeight =
            root.value(
                "tileHeight",
                0
            );

        const std::string
            tilesetPathUtf8 =
            root.value(
                "tileset",
                ""
            );

        if (width <= 0 ||
            height <= 0)
        {
            ENGINE_DEBUG_LOG(
                "Invalid TileMap size."
            );

            return nullptr;
        }

        if (tileWidth <= 0 ||
            tileHeight <= 0)
        {
            ENGINE_DEBUG_LOG(
                "Invalid TileMap tile size."
            );

            return nullptr;
        }

        if (tilesetPathUtf8.empty())
        {
            ENGINE_DEBUG_LOG(
                "TileMap Tileset path "
                "is empty."
            );

            return nullptr;
        }

        const std::wstring tilesetPath =
            Utf8ToWide(
                tilesetPathUtf8
            );

        if (tilesetPath.empty())
        {
            ENGINE_DEBUG_LOG(
                "Invalid Tileset path "
                "encoding."
            );

            return nullptr;
        }

        const auto layersIt =
            root.find(
                "layers"
            );

        if (layersIt ==
            root.end() ||
            !layersIt->is_array())
        {
            ENGINE_DEBUG_LOG(
                "TileMap layers must "
                "be an array."
            );

            return nullptr;
        }

        std::size_t expectedTileCount =
            0;

        if (!TryCalculateTileCount(
            width,
            height,
            expectedTileCount
        ))
        {
            ENGINE_DEBUG_LOG(
                "TileMap tile count "
                "overflow."
            );

            return nullptr;
        }

        auto data =
            std::make_unique<
            TileMapData
            >();

        data->width =
            width;

        data->height =
            height;

        data->tileWidth =
            tileWidth;

        data->tileHeight =
            tileHeight;

        data->tilesetPath =
            tilesetPath;

        for (const auto& layerJson :
            *layersIt)
        {
            if (!layerJson.is_object())
            {
                ENGINE_DEBUG_LOG(
                    "TileMap layer must "
                    "be an object."
                );

                return nullptr;
            }

            TileLayer layer;

            layer.name =
                layerJson.value(
                    "name",
                    "Unnamed"
                );

            const std::string typeName =
                layerJson.value(
                    "type",
                    ""
                );

            if (!TryParseLayerType(
                typeName,
                layer.type
            ))
            {
                ENGINE_DEBUG_LOG(
                    "Invalid TileMap "
                    "layer type."
                );

                return nullptr;
            }

            if (layer.type ==
                TileLayerType::Render)
            {
                const std::string
                    renderLayerName =
                    layerJson.value(
                        "renderLayer",
                        ""
                    );

                if (!TryParseRenderLayer(
                    renderLayerName,
                    layer.renderLayer
                ))
                {
                    ENGINE_DEBUG_LOG(
                        "Invalid TileMap "
                        "render layer."
                    );

                    return nullptr;
                }
            }
            else
            {
                layer.renderLayer =
                    RenderLayer::World;
            }

            layer.visible =
                layerJson.value(
                    "visible",
                    true
                );

            const auto tilesIt =
                layerJson.find(
                    "tiles"
                );

            if (tilesIt ==
                layerJson.end() ||
                !tilesIt->is_array())
            {
                ENGINE_DEBUG_LOG(
                    "TileMap layer is "
                    "missing tiles."
                );

                return nullptr;
            }

            layer.tiles =
                tilesIt->get<
                std::vector<TileId>
                >();

            if (layer.tiles.size() !=
                expectedTileCount)
            {
                ENGINE_DEBUG_LOG(
                    "TileMap layer tile "
                    "count mismatch."
                );

                return nullptr;
            }

            if (layer.type ==
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
                        ENGINE_DEBUG_LOG(
                            "Invalid collision "
                            "TileMap value."
                        );

                        return nullptr;
                    }
                }
            }

            data->layers.emplace_back(
                std::move(
                    layer
                )
            );
        }

        if (!Validate(
            *data
        ))
        {
            return nullptr;
        }

        return data;
    }
    catch (
        const nlohmann::json::exception& e
        )
    {
        ENGINE_DEBUG_LOG(
            e.what()
        );

        return nullptr;
    }
}

bool TileMapSerializer::Validate(
    const TileMapData& data
) noexcept
{
    if (data.width <= 0 ||
        data.height <= 0)
    {
        return false;
    }

    if (data.tileWidth <= 0 ||
        data.tileHeight <= 0)
    {
        return false;
    }

    if (data.tilesetPath.empty())
    {
        return false;
    }

    std::size_t expectedTileCount =
        0;

    if (!TryCalculateTileCount(
        data.width,
        data.height,
        expectedTileCount
    ))
    {
        return false;
    }

    for (const TileLayer& layer :
        data.layers)
    {
        if (layer.tiles.size() !=
            expectedTileCount)
        {
            return false;
        }

        if (layer.type ==
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
        else if (
            layer.type !=
            TileLayerType::Render)
        {
            return false;
        }
    }

    return true;
}