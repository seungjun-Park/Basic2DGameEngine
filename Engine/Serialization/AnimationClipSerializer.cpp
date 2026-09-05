#include "AnimationClipSerializer.h"

#include "Engine/Animation/AnimationClipData.h"
#include "Engine/Debug/DebugLog.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    bool IsValidUVRect(
        const UVRect& uv) noexcept
    {
        if (!std::isfinite(uv.u0) ||
            !std::isfinite(uv.v0) ||
            !std::isfinite(uv.u1) ||
            !std::isfinite(uv.v1))
        {
            return false;
        }

        if (uv.u0 < 0.0f ||
            uv.v0 < 0.0f ||
            uv.u1 > 1.0f ||
            uv.v1 > 1.0f)
        {
            return false;
        }

        if (uv.u0 >= uv.u1 ||
            uv.v0 >= uv.v1)
        {
            return false;
        }

        return true;
    }

    bool IsValidDuration(
        float duration) noexcept
    {
        return
            std::isfinite(duration) &&
            duration > 0.0f;
    }

    bool ReadUVRect(
        const nlohmann::json& json,
        UVRect& outUV)
    {
        if (!json.is_array() ||
            json.size() != 4)
        {
            return false;
        }

        for (const auto& value :
            json)
        {
            if (!value.is_number())
            {
                return false;
            }
        }

        outUV =
        {
            json[0].get<float>(),
            json[1].get<float>(),
            json[2].get<float>(),
            json[3].get<float>()
        };

        return
            IsValidUVRect(outUV);
    }

    std::wstring Utf8ToWide(
        const std::string& value)
    {
        if (value.empty())
        {
            return {};
        }

        const int requiredSize =
            ::MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                static_cast<int>(
                    value.size()),
                nullptr,
                0);

        if (requiredSize <= 0)
        {
            return {};
        }

        std::wstring result(
            static_cast<std::size_t>(
                requiredSize),
            L'\0');

        const int convertedSize =
            ::MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                static_cast<int>(
                    value.size()),
                result.data(),
                requiredSize);

        if (convertedSize !=
            requiredSize)
        {
            return {};
        }

        return result;
    }
}

std::unique_ptr<AnimationClipData>
AnimationClipSerializer::Load(
    const std::wstring& path)
{
    std::ifstream file
    {
        std::filesystem::path(
            path)
    };

    if (!file.is_open())
    {
        ANIMATIONCLIP_DEBUG_LOG(
            "Failed to open animation "
            "clip file.");

        return nullptr;
    }

    try
    {
        nlohmann::json root;

        file >> root;

        if (!root.is_object())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "AnimationClip root must "
                "be an object.");

            return nullptr;
        }

        const auto textureIt =
            root.find(
                "texture");

        if (textureIt ==
            root.end() ||
            !textureIt->is_string())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "AnimationClip texture "
                "path is missing.");

            return nullptr;
        }

        const std::string
            texturePathUtf8 =
            textureIt->
            get<std::string>();

        if (texturePathUtf8.empty())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "AnimationClip texture "
                "path is empty.");

            return nullptr;
        }

        const std::wstring
            texturePath =
            Utf8ToWide(
                texturePathUtf8);

        if (texturePath.empty())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "AnimationClip texture "
                "path is invalid UTF-8.");

            return nullptr;
        }

        bool looping = true;

        const auto loopingIt =
            root.find(
                "looping");

        if (loopingIt != root.end())
        {
            if (!loopingIt->is_boolean())
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "AnimationClip looping "
                    "must be boolean.");

                return nullptr;
            }

            looping =
                loopingIt->get<bool>();
        }

        const auto framesIt =
            root.find(
                "frames");

        if (framesIt ==
            root.end() ||
            !framesIt->is_array() ||
            framesIt->empty())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "AnimationClip frames must "
                "be a non-empty array.");

            return nullptr;
        }

        auto data =
            std::make_unique<
            AnimationClipData>();

        data->texturePath =
            texturePath;

        data->looping =
            looping;

        data->frames.reserve(
            framesIt->size());

        for (const auto& frameJson :
            *framesIt)
        {
            if (!frameJson.is_object())
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "AnimationClip frame "
                    "must be an object.");

                return nullptr;
            }

            const auto uvIt =
                frameJson.find(
                    "uv");

            if (uvIt ==
                frameJson.end())
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "AnimationClip frame UV "
                    "is missing.");

                return nullptr;
            }

            UVRect uv{};

            if (!ReadUVRect(
                *uvIt,
                uv))
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "AnimationClip frame UV "
                    "is invalid.");

                return nullptr;
            }

            const auto durationIt =
                frameJson.find(
                    "duration");

            if (durationIt ==
                frameJson.end() ||
                !durationIt->
                is_number())
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "AnimationClip frame "
                    "duration is missing "
                    "or invalid.");

                return nullptr;
            }

            const float duration =
                durationIt->
                get<float>();

            if (!IsValidDuration(
                duration))
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "AnimationClip frame "
                    "duration must be "
                    "positive and finite.");

                return nullptr;
            }

            AnimationClipFrameData
                frame{};

            frame.uv =
                uv;

            frame.duration =
                duration;

            data->frames.emplace_back(
                frame);
        }

        return data;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        ANIMATIONCLIP_DEBUG_LOG(
            e.what());

        return nullptr;
    }
}