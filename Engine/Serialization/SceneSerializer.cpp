#include "SceneSerializer.h"

#include "SceneData.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace
{
    void LogSceneError(
        const char* message)
    {
        OutputDebugStringA(
            "[SceneSerializer] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
    }

    const char* ToString(
        RenderLayer layer)
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
        }

        return nullptr;
    }

    bool ParseRenderLayer(
        const std::string& value,
        RenderLayer& outLayer)
    {
        if (value == "Background")
        {
            outLayer =
                RenderLayer::Background;
        }
        else if (value == "World")
        {
            outLayer =
                RenderLayer::World;
        }
        else if (value == "Effect")
        {
            outLayer =
                RenderLayer::Effect;
        }
        else if (value == "Foreground")
        {
            outLayer =
                RenderLayer::Foreground;
        }
        else if (value == "UI")
        {
            outLayer =
                RenderLayer::UI;
        }
        else if (value == "Debug")
        {
            outLayer =
                RenderLayer::Debug;
        }
        else
        {
            return false;
        }

        return true;
    }

    const char* ToString(
        BlendMode mode)
    {
        switch (mode)
        {
        case BlendMode::Opaque:
            return "Opaque";

        case BlendMode::Alpha:
            return "Alpha";
        }

        return nullptr;
    }

    bool ParseBlendMode(
        const std::string& value,
        BlendMode& outMode)
    {
        if (value == "Opaque")
        {
            outMode =
                BlendMode::Opaque;

            return true;
        }

        if (value == "Alpha")
        {
            outMode =
                BlendMode::Alpha;

            return true;
        }

        return false;
    }

    std::string WideToUtf8(
        const std::wstring& value)
    {
        if (value.empty())
        {
            return {};
        }

        const int size =
            WideCharToMultiByte(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(
                    value.size()
                    ),
                nullptr,
                0,
                nullptr,
                nullptr
            );

        if (size <= 0)
        {
            return {};
        }

        std::string result(
            size,
            '\0'
        );

        WideCharToMultiByte(
            CP_UTF8,
            0,
            value.data(),
            static_cast<int>(
                value.size()
                ),
            result.data(),
            size,
            nullptr,
            nullptr
        );

        return result;
    }

    std::wstring Utf8ToWide(
        const std::string& value)
    {
        if (value.empty())
        {
            return {};
        }

        const int size =
            MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value.data(),
                static_cast<int>(
                    value.size()
                    ),
                nullptr,
                0
            );

        if (size <= 0)
        {
            return {};
        }

        std::wstring result(
            size,
            L'\0'
        );

        MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(
                value.size()
                ),
            result.data(),
            size
        );

        return result;
    }

    bool IsValidUV(
        const UVRect& uv)
    {
        return
            std::isfinite(uv.u0) &&
            std::isfinite(uv.v0) &&
            std::isfinite(uv.u1) &&
            std::isfinite(uv.v1) &&

            uv.u0 >= 0.0f &&
            uv.v0 >= 0.0f &&
            uv.u1 <= 1.0f &&
            uv.v1 <= 1.0f &&

            uv.u0 < uv.u1 &&
            uv.v0 < uv.v1;
    }
}

bool SceneSerializer::Save(
    const SceneData& scene,
    const std::wstring& path)
{
    if (scene.version !=
        SceneData::CurrentVersion)
    {
        LogSceneError(
            "Unsupported scene version."
        );

        return false;
    }

    nlohmann::json root;

    root["version"] =
        scene.version;

    root["entities"] =
        nlohmann::json::array();

    for (const SerializedEntity& entity :
        scene.entities)
    {
        if (entity.type.empty())
        {
            LogSceneError(
                "Entity type is empty."
            );

            return false;
        }

        nlohmann::json entityJson;

        entityJson["type"] =
            entity.type;

        entityJson["active"] =
            entity.active;

        entityJson["transform"] =
        {
            {
                "position",
                {
                    entity.transform.position.x,
                    entity.transform.position.y
                }
            },
            {
                "scale",
                {
                    entity.transform.scale.x,
                    entity.transform.scale.y
                }
            },
            {
                "rotation",
                entity.transform.rotation
            }
        };

        if (entity.sprite)
        {
            const SerializedSprite&
                sprite =
                *entity.sprite;

            if (sprite.texturePath.empty() ||
                !IsValidUV(sprite.uv))
            {
                LogSceneError(
                    "Invalid sprite data."
                );

                return false;
            }

            const char* layer =
                ToString(
                    sprite.layer
                );

            const char* blend =
                ToString(
                    sprite.blendMode
                );

            if (!layer ||
                !blend)
            {
                LogSceneError(
                    "Invalid render state."
                );

                return false;
            }

            entityJson["sprite"] =
            {
                {
                    "texture",
                    WideToUtf8(
                        sprite.texturePath
                    )
                },
                {
                    "visible",
                    sprite.visible
                },
                {
                    "layer",
                    layer
                },
                {
                    "zIndex",
                    sprite.zIndex
                },
                {
                    "useYSort",
                    sprite.useYSort
                },
                {
                    "blendMode",
                    blend
                },
                {
                    "uv",
                    {
                        sprite.uv.u0,
                        sprite.uv.v0,
                        sprite.uv.u1,
                        sprite.uv.v1
                    }
                }
            };
        }

        root["entities"].
            push_back(
                std::move(
                    entityJson
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
        LogSceneError(
            "Failed to open output file."
        );

        return false;
    }

    file <<
        root.dump(4);

    return
        file.good();
}

std::unique_ptr<SceneData>
SceneSerializer::Load(
    const std::wstring& path)
{
    std::ifstream file
    {
        std::filesystem::path(
            path
        )
    };

    if (!file.is_open())
    {
        LogSceneError(
            "Failed to open scene file."
        );

        return nullptr;
    }

    try
    {
        nlohmann::json root;

        file >> root;

        if (!root.is_object())
        {
            return nullptr;
        }

        const std::uint32_t version =
            root.value(
                "version",
                0u
            );

        if (version !=
            SceneData::CurrentVersion)
        {
            LogSceneError(
                "Unsupported scene version."
            );

            return nullptr;
        }

        const auto entitiesIt =
            root.find(
                "entities"
            );

        if (entitiesIt ==
            root.end() ||
            !entitiesIt->is_array())
        {
            LogSceneError(
                "Entities must be an array."
            );

            return nullptr;
        }

        auto scene =
            std::make_unique<
            SceneData
            >();

        scene->version =
            version;

        for (const auto& entityJson :
            *entitiesIt)
        {
            if (!entityJson.is_object())
            {
                return nullptr;
            }

            SerializedEntity entity;

            entity.type =
                entityJson.value(
                    "type",
                    ""
                );

            if (entity.type.empty())
            {
                LogSceneError(
                    "Entity type is missing."
                );

                return nullptr;
            }

            entity.active =
                entityJson.value(
                    "active",
                    true
                );

            const auto transformIt =
                entityJson.find(
                    "transform"
                );

            if (transformIt ==
                entityJson.end() ||
                !transformIt->is_object())
            {
                return nullptr;
            }

            const auto positionIt =
                transformIt->find(
                    "position"
                );

            const auto scaleIt =
                transformIt->find(
                    "scale"
                );

            if (positionIt ==
                transformIt->end() ||
                scaleIt ==
                transformIt->end() ||
                !positionIt->is_array() ||
                !scaleIt->is_array() ||
                positionIt->size() != 2 ||
                scaleIt->size() != 2)
            {
                return nullptr;
            }

            entity.transform.position =
            {
                (*positionIt)[0].
                    get<float>(),

                (*positionIt)[1].
                    get<float>()
            };

            entity.transform.scale =
            {
                (*scaleIt)[0].
                    get<float>(),

                (*scaleIt)[1].
                    get<float>()
            };

            entity.transform.rotation =
                transformIt->value(
                    "rotation",
                    0.0f
                );

            const float values[]
            {
                entity.transform.position.x,
                entity.transform.position.y,
                entity.transform.scale.x,
                entity.transform.scale.y,
                entity.transform.rotation
            };

            for (float value : values)
            {
                if (!std::isfinite(value))
                {
                    LogSceneError(
                        "Transform contains "
                        "a non-finite value."
                    );

                    return nullptr;
                }
            }

            const auto spriteIt =
                entityJson.find(
                    "sprite"
                );

            if (spriteIt !=
                entityJson.end())
            {
                if (!spriteIt->is_object())
                {
                    return nullptr;
                }

                SerializedSprite sprite;

                const std::string texturePath =
                    spriteIt->value(
                        "texture",
                        ""
                    );

                if (texturePath.empty())
                {
                    return nullptr;
                }

                sprite.texturePath =
                    Utf8ToWide(
                        texturePath
                    );

                if (sprite.texturePath.empty())
                {
                    return nullptr;
                }

                sprite.visible =
                    spriteIt->value(
                        "visible",
                        true
                    );

                sprite.zIndex =
                    spriteIt->value(
                        "zIndex",
                        0.0f
                    );

                sprite.useYSort =
                    spriteIt->value(
                        "useYSort",
                        false
                    );

                if (!ParseRenderLayer(
                    spriteIt->value(
                        "layer",
                        ""
                    ),
                    sprite.layer))
                {
                    return nullptr;
                }

                if (!ParseBlendMode(
                    spriteIt->value(
                        "blendMode",
                        ""
                    ),
                    sprite.blendMode))
                {
                    return nullptr;
                }

                const auto uvIt =
                    spriteIt->find(
                        "uv"
                    );

                if (uvIt ==
                    spriteIt->end() ||
                    !uvIt->is_array() ||
                    uvIt->size() != 4)
                {
                    return nullptr;
                }

                sprite.uv =
                {
                    (*uvIt)[0].get<float>(),
                    (*uvIt)[1].get<float>(),
                    (*uvIt)[2].get<float>(),
                    (*uvIt)[3].get<float>()
                };

                if (!std::isfinite(
                    sprite.zIndex) ||
                    !IsValidUV(
                        sprite.uv))
                {
                    return nullptr;
                }

                entity.sprite =
                    std::move(
                        sprite
                    );
            }

            scene->entities.emplace_back(
                std::move(
                    entity
                )
            );
        }

        return scene;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        LogSceneError(
            e.what()
        );

        return nullptr;
    }
}