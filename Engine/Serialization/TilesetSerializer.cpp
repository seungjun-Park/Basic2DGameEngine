#include "TilesetSerializer.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Tile/TilesetData.h"

#include <Windows.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

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

    bool ReadOptionalNonNegativeInt(
        const nlohmann::json& root,
        const char* name,
        int& outValue
    )
    {
        const auto it =
            root.find(
                name
            );

        if (it ==
            root.end())
        {
            outValue = 0;

            return true;
        }

        if (!it->is_number_integer())
        {
            return false;
        }

        outValue =
            it->get<int>();

        return
            outValue >= 0;
    }
}

bool TilesetSerializer::Save(
    const TilesetData& data,
    const std::wstring& path
)
{
    if (!Validate(
        data
    ))
    {
        TILESET_DEBUG_LOG(
            "Cannot save invalid TilesetData."
        );

        return false;
    }

    if (path.empty())
    {
        TILESET_DEBUG_LOG(
            "Tileset output path is empty."
        );

        return false;
    }

    const std::string texturePath =
        WideToUtf8(
            data.texturePath
        );

    if (texturePath.empty())
    {
        TILESET_DEBUG_LOG(
            "Failed to encode Tileset "
            "texture path."
        );

        return false;
    }

    try
    {
        nlohmann::json root;

        root["texture"] =
            texturePath;

        root["tileWidth"] =
            data.tileWidth;

        root["tileHeight"] =
            data.tileHeight;

        root["columns"] =
            data.columns;

        root["rows"] =
            data.rows;

        root["margin"] =
            data.margin;

        root["spacing"] =
            data.spacing;

        std::ofstream file
        {
            std::filesystem::path(
                path
            )
        };

        if (!file.is_open())
        {
            TILESET_DEBUG_LOG(
                "Failed to open Tileset "
                "output file."
            );

            return false;
        }

        file <<
            root.dump(4);

        if (!file.good())
        {
            TILESET_DEBUG_LOG(
                "Failed to write Tileset."
            );

            return false;
        }

        return true;
    }
    catch (
        const nlohmann::json::exception& e
        )
    {
        TILESET_DEBUG_LOG(
            e.what()
        );

        return false;
    }
}

std::unique_ptr<TilesetData>
TilesetSerializer::Load(
    const std::wstring& path
)
{
    if (path.empty())
    {
        TILESET_DEBUG_LOG(
            "Tileset path is empty."
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
        TILESET_DEBUG_LOG(
            "Failed to open Tileset file."
        );

        return nullptr;
    }

    try
    {
        nlohmann::json root;

        file >> root;

        if (!root.is_object())
        {
            TILESET_DEBUG_LOG(
                "Tileset root must be "
                "a JSON object."
            );

            return nullptr;
        }

        const auto textureIt =
            root.find(
                "texture"
            );

        if (textureIt ==
            root.end() ||
            !textureIt->is_string())
        {
            TILESET_DEBUG_LOG(
                "Tileset texture path "
                "is missing or invalid."
            );

            return nullptr;
        }

        const std::string texturePathUtf8 =
            textureIt->get<
            std::string
            >();

        if (texturePathUtf8.empty())
        {
            TILESET_DEBUG_LOG(
                "Tileset texture path "
                "is empty."
            );

            return nullptr;
        }

        const std::wstring texturePath =
            Utf8ToWide(
                texturePathUtf8
            );

        if (texturePath.empty())
        {
            TILESET_DEBUG_LOG(
                "Tileset texture path "
                "is invalid UTF-8."
            );

            return nullptr;
        }

        const auto tileWidthIt =
            root.find(
                "tileWidth"
            );

        const auto tileHeightIt =
            root.find(
                "tileHeight"
            );

        const auto columnsIt =
            root.find(
                "columns"
            );

        const auto rowsIt =
            root.find(
                "rows"
            );

        if (tileWidthIt ==
            root.end() ||
            !tileWidthIt->
            is_number_integer() ||
            tileHeightIt ==
            root.end() ||
            !tileHeightIt->
            is_number_integer() ||
            columnsIt ==
            root.end() ||
            !columnsIt->
            is_number_integer() ||
            rowsIt ==
            root.end() ||
            !rowsIt->
            is_number_integer())
        {
            TILESET_DEBUG_LOG(
                "Tileset size fields "
                "are missing or invalid."
            );

            return nullptr;
        }

        auto data =
            std::make_unique<
            TilesetData
            >();

        data->texturePath =
            texturePath;

        data->tileWidth =
            tileWidthIt->get<int>();

        data->tileHeight =
            tileHeightIt->get<int>();

        data->columns =
            columnsIt->get<int>();

        data->rows =
            rowsIt->get<int>();

        if (!ReadOptionalNonNegativeInt(
            root,
            "margin",
            data->margin
        ))
        {
            TILESET_DEBUG_LOG(
                "Tileset margin is invalid."
            );

            return nullptr;
        }

        if (!ReadOptionalNonNegativeInt(
            root,
            "spacing",
            data->spacing
        ))
        {
            TILESET_DEBUG_LOG(
                "Tileset spacing is invalid."
            );

            return nullptr;
        }

        if (!Validate(
            *data
        ))
        {
            TILESET_DEBUG_LOG(
                "TilesetData is invalid."
            );

            return nullptr;
        }

        return data;
    }
    catch (
        const nlohmann::json::exception& e
        )
    {
        TILESET_DEBUG_LOG(
            e.what()
        );

        return nullptr;
    }
}

bool TilesetSerializer::Validate(
    const TilesetData& data
) noexcept
{
    if (data.texturePath.empty())
    {
        return false;
    }

    if (data.tileWidth <= 0 ||
        data.tileHeight <= 0)
    {
        return false;
    }

    if (data.columns <= 0 ||
        data.rows <= 0)
    {
        return false;
    }

    if (data.margin < 0 ||
        data.spacing < 0)
    {
        return false;
    }

    return true;
}