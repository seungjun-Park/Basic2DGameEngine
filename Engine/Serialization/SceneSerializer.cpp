#include "SceneSerializer.h"

#include "SceneData.h"

#include "Engine/Debug/DebugLog.h"

#include <Windows.h>

#include <nlohmann/json.hpp>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>

namespace
{
    const char* ToString(
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

    bool ParseRenderLayer(
        const std::string& value,
        RenderLayer& outLayer
    ) noexcept
    {
        if (value == "Background")
        {
            outLayer =
                RenderLayer::Background;

            return true;
        }

        if (value == "World")
        {
            outLayer =
                RenderLayer::World;

            return true;
        }

        if (value == "Effect")
        {
            outLayer =
                RenderLayer::Effect;

            return true;
        }

        if (value == "Foreground")
        {
            outLayer =
                RenderLayer::Foreground;

            return true;
        }

        if (value == "UI")
        {
            outLayer =
                RenderLayer::UI;

            return true;
        }

        if (value == "Debug")
        {
            outLayer =
                RenderLayer::Debug;

            return true;
        }

        return false;
    }

    const char* ToString(
        BlendMode mode
    ) noexcept
    {
        switch (mode)
        {
        case BlendMode::Opaque:
            return "Opaque";

        case BlendMode::Alpha:
            return "Alpha";

        default:
            return nullptr;
        }
    }

    bool ParseBlendMode(
        const std::string& value,
        BlendMode& outMode
    ) noexcept
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

    bool IsValidUV(
        const UVRect& uv
    ) noexcept
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

    bool IsValidVolume(
        float volume
    ) noexcept
    {
        return
            std::isfinite(volume) &&
            volume >= 0.0f &&
            volume <= 1.0f;
    }

    bool SaveAnimationBindings(
        const SceneData& scene,
        nlohmann::json& root
    )
    {
        root["animationBindings"] =
            nlohmann::json::array();

        std::unordered_set<std::string>
            slots;

        for (const SerializedAnimationBinding&
            binding :
            scene.animationBindings)
        {
            if (binding.slot.empty() ||
                binding.clipPath.empty())
            {
                ENGINE_DEBUG_LOG(
                    "Invalid animation binding."
                );

                return false;
            }

            if (!slots.insert(
                binding.slot
            ).second)
            {
                ENGINE_DEBUG_LOG(
                    "Duplicate animation binding slot."
                );

                return false;
            }

            const std::string clipPath =
                WideToUtf8(
                    binding.clipPath
                );

            if (clipPath.empty())
            {
                ENGINE_DEBUG_LOG(
                    "Failed to encode animation clip path."
                );

                return false;
            }

            root["animationBindings"].
                push_back(
                    {
                        {
                            "slot",
                            binding.slot
                        },
                        {
                            "clip",
                            clipPath
                        }
                    }
                );
        }

        return true;
    }

    bool SaveAudioBindings(
        const SceneData& scene,
        nlohmann::json& root
    )
    {
        root["audioBindings"] =
            nlohmann::json::array();

        std::unordered_set<std::string>
            slots;

        for (const SerializedAudioBinding&
            binding :
            scene.audioBindings)
        {
            if (binding.slot.empty() ||
                binding.clipPath.empty() ||
                !IsValidVolume(
                    binding.volume
                ))
            {
                ENGINE_DEBUG_LOG(
                    "Invalid audio binding."
                );

                return false;
            }

            if (!slots.insert(
                binding.slot
            ).second)
            {
                ENGINE_DEBUG_LOG(
                    "Duplicate audio binding slot."
                );

                return false;
            }

            const std::string clipPath =
                WideToUtf8(
                    binding.clipPath
                );

            if (clipPath.empty())
            {
                ENGINE_DEBUG_LOG(
                    "Failed to encode audio clip path."
                );

                return false;
            }

            root["audioBindings"].
                push_back(
                    {
                        {
                            "slot",
                            binding.slot
                        },
                        {
                            "clip",
                            clipPath
                        },
                        {
                            "volume",
                            binding.volume
                        }
                    }
                );
        }

        return true;
    }

    bool SaveEntities(
        const SceneData& scene,
        nlohmann::json& root
    )
    {
        root["entities"] =
            nlohmann::json::array();

        for (const SerializedEntity& entity :
            scene.entities)
        {
            if (entity.type.empty())
            {
                ENGINE_DEBUG_LOG(
                    "Entity type is empty."
                );

                return false;
            }

            if (!std::isfinite(
                entity.transform.position.x
            ) ||
                !std::isfinite(
                    entity.transform.position.y
                ) ||
                !std::isfinite(
                    entity.transform.scale.x
                ) ||
                !std::isfinite(
                    entity.transform.scale.y
                ) ||
                !std::isfinite(
                    entity.transform.rotation
                ))
            {
                ENGINE_DEBUG_LOG(
                    "Transform contains non-finite values."
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
                const SerializedSprite& sprite =
                    *entity.sprite;

                if (sprite.texturePath.empty() ||
                    !std::isfinite(
                        sprite.zIndex
                    ) ||
                    !IsValidUV(
                        sprite.uv
                    ))
                {
                    ENGINE_DEBUG_LOG(
                        "Invalid sprite data."
                    );

                    return false;
                }

                const std::string texturePath =
                    WideToUtf8(
                        sprite.texturePath
                    );

                if (texturePath.empty())
                {
                    ENGINE_DEBUG_LOG(
                        "Failed to encode sprite texture path."
                    );

                    return false;
                }

                const char* renderLayer =
                    ToString(
                        sprite.layer
                    );

                const char* blendMode =
                    ToString(
                        sprite.blendMode
                    );

                if (!renderLayer ||
                    !blendMode)
                {
                    ENGINE_DEBUG_LOG(
                        "Invalid sprite render state."
                    );

                    return false;
                }

                entityJson["sprite"] =
                {
                    {
                        "texture",
                        texturePath
                    },
                    {
                        "visible",
                        sprite.visible
                    },
                    {
                        "layer",
                        renderLayer
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
                        blendMode
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

            if (!entity.animationClipPath.empty())
            {
                const std::string animationClipPath =
                    WideToUtf8(
                        entity.animationClipPath
                    );

                if (animationClipPath.empty())
                {
                    ENGINE_DEBUG_LOG(
                        "Failed to encode entity "
                        "AnimationClip path."
                    );

                    return false;
                }

                entityJson["animationClip"] =
                    animationClipPath;
            }

            root["entities"].
                push_back(
                    std::move(
                        entityJson
                    )
                );
        }

        return true;
    }

    bool LoadAnimationBindings(
        const nlohmann::json& root,
        SceneData& scene
    )
    {
        const auto bindingsIt =
            root.find(
                "animationBindings"
            );

        if (bindingsIt ==
            root.end() ||
            !bindingsIt->is_array())
        {
            ENGINE_DEBUG_LOG(
                "Animation bindings must be an array."
            );

            return false;
        }

        std::unordered_set<std::string>
            slots;

        for (const auto& bindingJson :
            *bindingsIt)
        {
            if (!bindingJson.is_object())
            {
                return false;
            }

            const auto slotIt =
                bindingJson.find(
                    "slot"
                );

            const auto clipIt =
                bindingJson.find(
                    "clip"
                );

            if (slotIt ==
                bindingJson.end() ||
                !slotIt->is_string() ||
                clipIt ==
                bindingJson.end() ||
                !clipIt->is_string())
            {
                return false;
            }

            SerializedAnimationBinding
                binding;

            binding.slot =
                slotIt->get<
                std::string
                >();

            const std::string clipPath =
                clipIt->get<
                std::string
                >();

            if (binding.slot.empty() ||
                clipPath.empty())
            {
                return false;
            }

            if (!slots.insert(
                binding.slot
            ).second)
            {
                return false;
            }

            binding.clipPath =
                Utf8ToWide(
                    clipPath
                );

            if (binding.clipPath.empty())
            {
                return false;
            }

            scene.animationBindings.
                emplace_back(
                    std::move(
                        binding
                    )
                );
        }

        return true;
    }

    bool LoadAudioBindings(
        const nlohmann::json& root,
        SceneData& scene
    )
    {
        const auto bindingsIt =
            root.find(
                "audioBindings"
            );

        if (bindingsIt ==
            root.end() ||
            !bindingsIt->is_array())
        {
            ENGINE_DEBUG_LOG(
                "Audio bindings must be an array."
            );

            return false;
        }

        std::unordered_set<std::string>
            slots;

        for (const auto& bindingJson :
            *bindingsIt)
        {
            if (!bindingJson.is_object())
            {
                return false;
            }

            const auto slotIt =
                bindingJson.find(
                    "slot"
                );

            const auto clipIt =
                bindingJson.find(
                    "clip"
                );

            const auto volumeIt =
                bindingJson.find(
                    "volume"
                );

            if (slotIt ==
                bindingJson.end() ||
                !slotIt->is_string() ||
                clipIt ==
                bindingJson.end() ||
                !clipIt->is_string() ||
                volumeIt ==
                bindingJson.end() ||
                !volumeIt->is_number())
            {
                return false;
            }

            SerializedAudioBinding binding;

            binding.slot =
                slotIt->get<
                std::string
                >();

            const std::string clipPath =
                clipIt->get<
                std::string
                >();

            binding.volume =
                volumeIt->get<float>();

            if (binding.slot.empty() ||
                clipPath.empty() ||
                !IsValidVolume(
                    binding.volume
                ))
            {
                return false;
            }

            if (!slots.insert(
                binding.slot
            ).second)
            {
                return false;
            }

            binding.clipPath =
                Utf8ToWide(
                    clipPath
                );

            if (binding.clipPath.empty())
            {
                return false;
            }

            scene.audioBindings.
                emplace_back(
                    std::move(
                        binding
                    )
                );
        }

        return true;
    }

    bool LoadEntities(
        const nlohmann::json& root,
        SceneData& scene,
        bool loadAnimationAssignments
    )
    {
        const auto entitiesIt =
            root.find(
                "entities"
            );

        if (entitiesIt ==
            root.end() ||
            !entitiesIt->is_array())
        {
            ENGINE_DEBUG_LOG(
                "Entities must be an array."
            );

            return false;
        }

        for (const auto& entityJson :
            *entitiesIt)
        {
            if (!entityJson.is_object())
            {
                return false;
            }

            const auto typeIt =
                entityJson.find(
                    "type"
                );

            if (typeIt ==
                entityJson.end() ||
                !typeIt->is_string())
            {
                return false;
            }

            SerializedEntity entity;

            entity.type =
                typeIt->get<
                std::string
                >();

            if (entity.type.empty())
            {
                return false;
            }

            const auto activeIt =
                entityJson.find(
                    "active"
                );

            if (activeIt !=
                entityJson.end())
            {
                if (!activeIt->is_boolean())
                {
                    return false;
                }

                entity.active =
                    activeIt->get<bool>();
            }

            const auto transformIt =
                entityJson.find(
                    "transform"
                );

            if (transformIt ==
                entityJson.end() ||
                !transformIt->is_object())
            {
                return false;
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
                return false;
            }

            entity.transform.position =
            {
                (*positionIt)[0].get<float>(),
                (*positionIt)[1].get<float>()
            };

            entity.transform.scale =
            {
                (*scaleIt)[0].get<float>(),
                (*scaleIt)[1].get<float>()
            };

            const auto rotationIt =
                transformIt->find(
                    "rotation"
                );

            if (rotationIt !=
                transformIt->end())
            {
                if (!rotationIt->is_number())
                {
                    return false;
                }

                entity.transform.rotation =
                    rotationIt->get<float>();
            }

            if (!std::isfinite(
                entity.transform.position.x
            ) ||
                !std::isfinite(
                    entity.transform.position.y
                ) ||
                !std::isfinite(
                    entity.transform.scale.x
                ) ||
                !std::isfinite(
                    entity.transform.scale.y
                ) ||
                !std::isfinite(
                    entity.transform.rotation
                ))
            {
                return false;
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
                    return false;
                }

                SerializedSprite sprite;

                const auto textureIt =
                    spriteIt->find(
                        "texture"
                    );

                if (textureIt ==
                    spriteIt->end() ||
                    !textureIt->is_string())
                {
                    return false;
                }

                const std::string texturePath =
                    textureIt->get<
                    std::string
                    >();

                if (texturePath.empty())
                {
                    return false;
                }

                sprite.texturePath =
                    Utf8ToWide(
                        texturePath
                    );

                if (sprite.texturePath.empty())
                {
                    return false;
                }

                const auto visibleIt =
                    spriteIt->find(
                        "visible"
                    );

                if (visibleIt !=
                    spriteIt->end())
                {
                    if (!visibleIt->is_boolean())
                    {
                        return false;
                    }

                    sprite.visible =
                        visibleIt->get<bool>();
                }

                const auto zIndexIt =
                    spriteIt->find(
                        "zIndex"
                    );

                if (zIndexIt !=
                    spriteIt->end())
                {
                    if (!zIndexIt->is_number())
                    {
                        return false;
                    }

                    sprite.zIndex =
                        zIndexIt->get<float>();
                }

                const auto useYSortIt =
                    spriteIt->find(
                        "useYSort"
                    );

                if (useYSortIt !=
                    spriteIt->end())
                {
                    if (!useYSortIt->is_boolean())
                    {
                        return false;
                    }

                    sprite.useYSort =
                        useYSortIt->get<bool>();
                }

                const auto layerIt =
                    spriteIt->find(
                        "layer"
                    );

                const auto blendModeIt =
                    spriteIt->find(
                        "blendMode"
                    );

                if (layerIt ==
                    spriteIt->end() ||
                    !layerIt->is_string() ||
                    blendModeIt ==
                    spriteIt->end() ||
                    !blendModeIt->is_string())
                {
                    return false;
                }

                if (!ParseRenderLayer(
                    layerIt->get<
                    std::string
                    >(),
                    sprite.layer
                ))
                {
                    return false;
                }

                if (!ParseBlendMode(
                    blendModeIt->get<
                    std::string
                    >(),
                    sprite.blendMode
                ))
                {
                    return false;
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
                    return false;
                }

                sprite.uv =
                {
                    (*uvIt)[0].get<float>(),
                    (*uvIt)[1].get<float>(),
                    (*uvIt)[2].get<float>(),
                    (*uvIt)[3].get<float>()
                };

                if (!std::isfinite(
                    sprite.zIndex
                ) ||
                    !IsValidUV(
                        sprite.uv
                    ))
                {
                    return false;
                }

                entity.sprite =
                    std::move(
                        sprite
                    );
            }

            if (loadAnimationAssignments)
            {
                const auto animationIt =
                    entityJson.find(
                        "animationClip"
                    );

                if (animationIt !=
                    entityJson.end())
                {
                    if (!animationIt->is_string())
                    {
                        return false;
                    }

                    const std::string
                        animationPath =
                        animationIt->get<
                        std::string
                        >();

                    if (animationPath.empty())
                    {
                        return false;
                    }

                    entity.animationClipPath =
                        Utf8ToWide(
                            animationPath
                        );

                    if (entity.animationClipPath.empty())
                    {
                        return false;
                    }
                }
            }

            scene.entities.emplace_back(
                std::move(
                    entity
                )
            );
        }

        return true;
    }
}

bool SceneSerializer::Save(
    const SceneData& scene,
    const std::wstring& path
)
{
    if (scene.version !=
        SceneData::CurrentVersion)
    {
        ENGINE_DEBUG_LOG(
            "Unsupported scene version."
        );

        return false;
    }

    if (path.empty())
    {
        ENGINE_DEBUG_LOG(
            "Scene output path is empty."
        );

        return false;
    }

    try
    {
        nlohmann::json root;

        root["version"] =
            scene.version;

        const std::string tileMapPath =
            WideToUtf8(
                scene.tileMapPath
            );

        if (!scene.tileMapPath.empty() &&
            tileMapPath.empty())
        {
            return false;
        }

        root["tileMap"] =
            tileMapPath;

        if (!SaveAnimationBindings(
            scene,
            root
        ))
        {
            return false;
        }

        if (!SaveAudioBindings(
            scene,
            root
        ))
        {
            return false;
        }

        if (!SaveEntities(
            scene,
            root
        ))
        {
            return false;
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
                "Failed to open scene output file."
            );

            return false;
        }

        file <<
            root.dump(4);

        return
            file.good();
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

std::unique_ptr<SceneData>
SceneSerializer::Load(
    const std::wstring& path
)
{
    if (path.empty())
    {
        ENGINE_DEBUG_LOG(
            "Scene path is empty."
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

        const auto versionIt =
            root.find(
                "version"
            );

        if (versionIt ==
            root.end() ||
            !versionIt->
            is_number_unsigned())
        {
            return nullptr;
        }

        const std::uint32_t version =
            versionIt->get<
            std::uint32_t
            >();

        const bool supported =
            version ==
            SceneData::LegacyVersion ||
            version ==
            SceneData::ResourceBindingVersion ||
            version ==
            SceneData::AudioBindingVersion ||
            version ==
            SceneData::CurrentVersion;

        if (!supported)
        {
            ENGINE_DEBUG_LOG(
                "Unsupported scene version."
            );

            return nullptr;
        }

        auto scene =
            std::make_unique<
            SceneData
            >();

        scene->version =
            SceneData::CurrentVersion;

        if (version >=
            SceneData::ResourceBindingVersion)
        {
            const auto tileMapIt =
                root.find(
                    "tileMap"
                );

            if (tileMapIt ==
                root.end() ||
                !tileMapIt->is_string())
            {
                return nullptr;
            }

            const std::string tileMapPath =
                tileMapIt->get<
                std::string
                >();

            if (!tileMapPath.empty())
            {
                scene->tileMapPath =
                    Utf8ToWide(
                        tileMapPath
                    );

                if (scene->tileMapPath.empty())
                {
                    return nullptr;
                }
            }

            if (!LoadAnimationBindings(
                root,
                *scene
            ))
            {
                return nullptr;
            }
        }

        if (version >=
            SceneData::AudioBindingVersion)
        {
            if (!LoadAudioBindings(
                root,
                *scene
            ))
            {
                return nullptr;
            }
        }

        if (!LoadEntities(
            root,
            *scene,
            version >=
            SceneData::CurrentVersion
        ))
        {
            return nullptr;
        }

        return scene;
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