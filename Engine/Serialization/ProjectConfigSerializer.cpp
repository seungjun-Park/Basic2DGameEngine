#include "ProjectConfigSerializer.h"

#include "Engine/Core/ProjectConfig.h"
#include "Engine/Debug/DebugLog.h"

#include <nlohmann/json.hpp>

#include <Windows.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>


namespace
{
    const char* ToString(
        WindowMode mode)
    {
        switch (mode)
        {
        case WindowMode::Windowed:
            return "Windowed";

        case WindowMode::BorderlessFullscreen:
            return "BorderlessFullscreen";
        }

        return nullptr;
    }

    bool ParseWindowMode(
        const std::string& value,
        WindowMode& outMode)
    {
        if (value == "Windowed")
        {
            outMode =
                WindowMode::Windowed;

            return true;
        }

        if (value ==
            "BorderlessFullscreen")
        {
            outMode =
                WindowMode::BorderlessFullscreen;

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

        const int converted =
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

        if (converted != size)
        {
            return {};
        }

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

        const int converted =
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

        if (converted != size)
        {
            return {};
        }

        return result;
    }

    bool ValidateConfig(
        const ProjectConfig& config)
    {
        if (config.version !=
            ProjectConfig::CurrentVersion)
        {
            ENGINE_DEBUG_LOG(
                "Unsupported project version."
            );

            return false;
        }

        const EngineConfig& engine =
            config.engine;

        if (engine.windowTitle.empty())
        {
            ENGINE_DEBUG_LOG(
                "Window title is empty."
            );

            return false;
        }

        if (engine.windowWidth <= 0 ||
            engine.windowHeight <= 0)
        {
            ENGINE_DEBUG_LOG(
                "Invalid window size."
            );

            return false;
        }

        if (!std::isfinite(
            engine.fixedUpdateHz) ||
            engine.fixedUpdateHz <= 0.0f)
        {
            ENGINE_DEBUG_LOG(
                "Invalid fixed update rate."
            );

            return false;
        }

        if (engine.maxFixedSteps == 0)
        {
            ENGINE_DEBUG_LOG(
                "maxFixedSteps must be greater than zero."
            );

            return false;
        }

        if (!std::isfinite(
            engine.maxDeltaTime) ||
            engine.maxDeltaTime <= 0.0f)
        {
            ENGINE_DEBUG_LOG(
                "Invalid maxDeltaTime."
            );

            return false;
        }

        if (config.assetRoot.empty())
        {
            ENGINE_DEBUG_LOG(
                "Asset root is empty."
            );

            return false;
        }

        if (!ToString(
            engine.windowMode))
        {
            ENGINE_DEBUG_LOG(
                "Invalid window mode."
            );

            return false;
        }

        return true;
    }
}

bool ProjectConfigSerializer::Save(
    const ProjectConfig& config,
    const std::wstring& path)
{
    if (!ValidateConfig(
        config))
    {
        return false;
    }

    const char* windowMode =
        ToString(
            config.engine.windowMode
        );

    if (!windowMode)
    {
        return false;
    }

    const std::string windowTitle =
        WideToUtf8(
            config.engine.windowTitle
        );

    const std::string assetRoot =
        WideToUtf8(
            config.assetRoot
        );

    const std::string startScene =
        WideToUtf8(
            config.startScene
        );

    if (windowTitle.empty() ||
        assetRoot.empty())
    {
        ENGINE_DEBUG_LOG(
            "Failed to convert project strings to UTF-8."
        );

        return false;
    }

    if (!config.startScene.empty() &&
        startScene.empty())
    {
        ENGINE_DEBUG_LOG(
            "Failed to convert start scene path to UTF-8."
        );

        return false;
    }

    try
    {
        nlohmann::json root;

        root["version"] =
            config.version;

        root["window"] =
        {
            {
                "title",
                windowTitle
            },
            {
                "width",
                config.engine.windowWidth
            },
            {
                "height",
                config.engine.windowHeight
            },
            {
                "mode",
                windowMode
            },
            {
                "pauseWhenUnfocused",
                config.engine.pauseWhenUnfocused
            }
        };

        root["rendering"] =
        {
            {
                "vsync",
                config.engine.vsync
            },
            {
                "targetFPS",
                config.engine.targetFPS
            }
        };

        root["fixedUpdate"] =
        {
            {
                "hz",
                config.engine.fixedUpdateHz
            },
            {
                "maxSteps",
                config.engine.maxFixedSteps
            },
            {
                "maxDeltaTime",
                config.engine.maxDeltaTime
            }
        };

        root["debug"] =
        {
            {
                "showCollider",
                config.engine.showDebugCollider
            },
            {
                "showRuntimeStats",
                config.engine.showRuntimeStats
            }
        };

        root["project"] =
        {
            {
                "assetRoot",
                assetRoot
            },
            {
                "startScene",
                startScene
            }
        };

        std::ofstream file
        {
            std::filesystem::path(
                path
            )
        };

        if (!file.is_open())
        {
            ENGINE_DEBUG_LOG(
                "Failed to open project config output file."
            );

            return false;
        }

        file <<
            root.dump(4);

        if (!file.good())
        {
            ENGINE_DEBUG_LOG(
                "Failed to write project config."
            );

            return false;
        }

        return true;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        ENGINE_DEBUG_LOG(
            e.what()
        );

        return false;
    }
}

std::unique_ptr<ProjectConfig>
ProjectConfigSerializer::Load(
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
        ENGINE_DEBUG_LOG(
            "Failed to open project config file."
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
                "Project config root must be an object."
            );

            return nullptr;
        }

        const std::uint32_t version =
            root.value(
                "version",
                0u
            );

        if (version !=
            ProjectConfig::CurrentVersion)
        {
            ENGINE_DEBUG_LOG(
                "Unsupported project config version."
            );

            return nullptr;
        }

        const auto windowIt =
            root.find(
                "window"
            );

        const auto renderingIt =
            root.find(
                "rendering"
            );

        const auto fixedUpdateIt =
            root.find(
                "fixedUpdate"
            );

        const auto debugIt =
            root.find(
                "debug"
            );

        const auto projectIt =
            root.find(
                "project"
            );

        if (windowIt == root.end() ||
            !windowIt->is_object() ||
            renderingIt == root.end() ||
            !renderingIt->is_object() ||
            fixedUpdateIt == root.end() ||
            !fixedUpdateIt->is_object() ||
            debugIt == root.end() ||
            !debugIt->is_object() ||
            projectIt == root.end() ||
            !projectIt->is_object())
        {
            ENGINE_DEBUG_LOG(
                "Project config contains missing or invalid sections."
            );

            return nullptr;
        }

        auto config =
            std::make_unique<
            ProjectConfig
            >();

        config->version =
            version;

        //
        // Window
        //

        const std::string title =
            windowIt->value(
                "title",
                ""
            );

        if (title.empty())
        {
            ENGINE_DEBUG_LOG(
                "Window title is missing."
            );

            return nullptr;
        }

        config->engine.windowTitle =
            Utf8ToWide(
                title
            );

        if (config->
            engine.windowTitle.empty())
        {
            ENGINE_DEBUG_LOG(
                "Invalid UTF-8 window title."
            );

            return nullptr;
        }

        config->engine.windowWidth =
            windowIt->value(
                "width",
                0
            );

        config->engine.windowHeight =
            windowIt->value(
                "height",
                0
            );

        if (!ParseWindowMode(
            windowIt->value(
                "mode",
                ""
            ),
            config->engine.windowMode))
        {
            ENGINE_DEBUG_LOG(
                "Invalid window mode."
            );

            return nullptr;
        }

        config->
            engine.pauseWhenUnfocused =
            windowIt->value(
                "pauseWhenUnfocused",
                true
            );

        //
        // Rendering
        //

        config->engine.vsync =
            renderingIt->value(
                "vsync",
                false
            );

        config->engine.targetFPS =
            renderingIt->value(
                "targetFPS",
                0u
            );

        //
        // Fixed Update
        //

        config->engine.fixedUpdateHz =
            fixedUpdateIt->value(
                "hz",
                0.0f
            );

        config->engine.maxFixedSteps =
            fixedUpdateIt->value(
                "maxSteps",
                0u
            );

        config->engine.maxDeltaTime =
            fixedUpdateIt->value(
                "maxDeltaTime",
                0.0f
            );

        //
        // Debug
        //

        config->
            engine.showDebugCollider =
            debugIt->value(
                "showCollider",
                true
            );

        config->
            engine.showRuntimeStats =
            debugIt->value(
                "showRuntimeStats",
                true
            );

        //
        // Project
        //

        const std::string assetRoot =
            projectIt->value(
                "assetRoot",
                ""
            );

        if (assetRoot.empty())
        {
            ENGINE_DEBUG_LOG(
                "Asset root is missing."
            );

            return nullptr;
        }

        config->assetRoot =
            Utf8ToWide(
                assetRoot
            );

        if (config->assetRoot.empty())
        {
            ENGINE_DEBUG_LOG(
                "Invalid UTF-8 asset root."
            );

            return nullptr;
        }

        const std::string startScene =
            projectIt->value(
                "startScene",
                ""
            );

        if (!startScene.empty())
        {
            config->startScene =
                Utf8ToWide(
                    startScene
                );

            if (config->
                startScene.empty())
            {
                ENGINE_DEBUG_LOG(
                    "Invalid UTF-8 start scene path."
                );

                return nullptr;
            }
        }
        else
        {
            config->
                startScene.clear();
        }

        if (!ValidateConfig(
            *config))
        {
            return nullptr;
        }

        return config;
    }
    catch (
        const nlohmann::json::exception& e)
    {
        ENGINE_DEBUG_LOG(
            e.what()
        );

        return nullptr;
    }
}