#include "AnimationClipLoader.h"

#include "AnimationClip.h"

#include "Engine/Resource/ResourceManager.h"
#include "Engine/Graphics/Texture.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    void LogAnimationClipError(
        const char* message)
    {
        OutputDebugStringA(
            "[AnimationClipLoader] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
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
        LogAnimationClipError(
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
            LogAnimationClipError(
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
            LogAnimationClipError(
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
            LogAnimationClipError(
                "Frames must be a non-empty array."
            );

            return nullptr;
        }

        //
        // 현재 project asset path contract와
        // 동일하게 ASCII-compatible relative path를 사용한다.
        //
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
            LogAnimationClipError(
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
                LogAnimationClipError(
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
                LogAnimationClipError(
                    "Frame UV is missing."
                );

                return nullptr;
            }

            UVRect uv{};

            if (!ReadUVRect(
                *uvIt,
                uv))
            {
                LogAnimationClipError(
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

            //
            // UV range와 duration 최종 validation은
            // AnimationClip 자체의 contract를 재사용한다.
            //
            if (!clip->AddFrame(
                uv,
                duration))
            {
                LogAnimationClipError(
                    "Invalid animation frame."
                );

                return nullptr;
            }
        }

        if (!clip->IsValid())
        {
            LogAnimationClipError(
                "Animation clip is invalid."
            );

            return nullptr;
        }

        return clip;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        LogAnimationClipError(
            e.what()
        );

        return nullptr;
    }
}