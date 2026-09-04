#include "AnimationClipLoader.h"

#include "AnimationClip.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Resource/ResourceManager.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>


namespace
{
    bool ReadUVRect(
        const nlohmann::json& json,
        UVRect& outUV)
    {
        if (!json.is_array() ||
            json.size() != 4)
        {
            return false;
        }

        for (const auto& value : json)
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

        return true;
    }
}

std::unique_ptr<AnimationClip>
AnimationClipLoader::Load(
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
        ANIMATIONCLIP_DEBUG_LOG(
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
            ANIMATIONCLIP_DEBUG_LOG(
                "Root must be a JSON object."
            );

            return nullptr;
        }

        const std::string texturePath =
            json.value(
                "texture",
                ""
            );

        if (texturePath.empty())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "Texture path is empty."
            );

            return nullptr;
        }

        const bool looping =
            json.value(
                "looping",
                true
            );

        const auto framesIt =
            json.find(
                "frames"
            );

        if (framesIt == json.end() ||
            !framesIt->is_array() ||
            framesIt->empty())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "Frames must be a non-empty array."
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
            ANIMATIONCLIP_DEBUG_LOG(
                "Failed to load texture."
            );

            return nullptr;
        }

        auto clip =
            std::make_unique<
            AnimationClip
            >();

        clip->SetTexture(
            texture
        );

        clip->SetLooping(
            looping
        );

        for (const auto& frameJson :
            *framesIt)
        {
            if (!frameJson.is_object())
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "Frame must be an object."
                );

                return nullptr;
            }

            const auto uvIt =
                frameJson.find(
                    "uv"
                );

            if (uvIt ==
                frameJson.end())
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "Frame UV is missing."
                );

                return nullptr;
            }

            UVRect uv{};

            if (!ReadUVRect(
                *uvIt,
                uv))
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "Frame UV must contain "
                    "four numeric values."
                );

                return nullptr;
            }

            const float duration =
                frameJson.value(
                    "duration",
                    0.0f
                );

            if (!clip->AddFrame(
                uv,
                duration))
            {
                ANIMATIONCLIP_DEBUG_LOG(
                    "Invalid animation frame."
                );

                return nullptr;
            }
        }

        if (!clip->IsValid())
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "Animation clip is invalid."
            );

            return nullptr;
        }

        return clip;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        ANIMATIONCLIP_DEBUG_LOG(
            e.what()
        );

        return nullptr;
    }
}