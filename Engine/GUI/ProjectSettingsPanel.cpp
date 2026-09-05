#include "ProjectSettingsPanel.h"

#include "Engine/Serialization/ProjectConfigSerializer.h"

#include <imgui.h>

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

namespace
{
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
                    value.size()),
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
            static_cast<std::size_t>(size),
            '\0'
        );

        const int converted =
            WideCharToMultiByte(
                CP_UTF8,
                0,
                value.data(),
                static_cast<int>(
                    value.size()),
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
        const char* value)
    {
        if (!value ||
            value[0] == '\0')
        {
            return {};
        }

        const int inputLength =
            static_cast<int>(
                std::strlen(value));

        const int size =
            MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value,
                inputLength,
                nullptr,
                0
            );

        if (size <= 0)
        {
            return {};
        }

        std::wstring result(
            static_cast<std::size_t>(size),
            L'\0'
        );

        const int converted =
            MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                value,
                inputLength,
                result.data(),
                size
            );

        if (converted != size)
        {
            return {};
        }

        return result;
    }

    template <std::size_t N>
    bool CopyWideToBuffer(
        const std::wstring& value,
        std::array<char, N>& buffer)
    {
        const std::string utf8 =
            WideToUtf8(value);

        if (!value.empty() &&
            utf8.empty())
        {
            return false;
        }

        if (utf8.size() >=
            buffer.size())
        {
            return false;
        }

        buffer.fill('\0');

        if (!utf8.empty())
        {
            std::memcpy(
                buffer.data(),
                utf8.data(),
                utf8.size()
            );
        }

        return true;
    }

    template <std::size_t N>
    bool ApplyBuffer(
        const std::array<char, N>& buffer,
        std::wstring& outValue)
    {
        if (buffer[0] == '\0')
        {
            outValue.clear();
            return true;
        }

        std::wstring converted =
            Utf8ToWide(
                buffer.data());

        if (converted.empty())
        {
            return false;
        }

        outValue =
            std::move(converted);

        return true;
    }

    bool SameEngineConfig(
        const EngineConfig& lhs,
        const EngineConfig& rhs)
    {
        return
            lhs.windowTitle ==
            rhs.windowTitle &&

            lhs.windowWidth ==
            rhs.windowWidth &&

            lhs.windowHeight ==
            rhs.windowHeight &&

            lhs.windowMode ==
            rhs.windowMode &&

            lhs.vsync ==
            rhs.vsync &&

            lhs.targetFPS ==
            rhs.targetFPS &&

            lhs.fixedUpdateHz ==
            rhs.fixedUpdateHz &&

            lhs.maxFixedSteps ==
            rhs.maxFixedSteps &&

            lhs.maxDeltaTime ==
            rhs.maxDeltaTime &&

            lhs.pauseWhenUnfocused ==
            rhs.pauseWhenUnfocused &&

            lhs.showDebugCollider ==
            rhs.showDebugCollider &&

            lhs.showRuntimeStats ==
            rhs.showRuntimeStats;
    }

    bool SameProjectConfig(
        const ProjectConfig& lhs,
        const ProjectConfig& rhs)
    {
        return
            lhs.version ==
            rhs.version &&

            SameEngineConfig(
                lhs.engine,
                rhs.engine) &&

            lhs.assetRoot ==
            rhs.assetRoot &&

            lhs.startScene ==
            rhs.startScene;
    }

    bool RequiresRestart(
        const ProjectConfig& draft,
        const ProjectConfig& startup)
    {
        const EngineConfig& current =
            draft.engine;

        const EngineConfig& initial =
            startup.engine;

        return
            current.windowWidth !=
            initial.windowWidth ||

            current.windowHeight !=
            initial.windowHeight ||

            current.windowMode !=
            initial.windowMode ||

            current.fixedUpdateHz !=
            initial.fixedUpdateHz ||

            current.maxFixedSteps !=
            initial.maxFixedSteps ||

            current.maxDeltaTime !=
            initial.maxDeltaTime ||

            current.showDebugCollider !=
            initial.showDebugCollider ||

            draft.assetRoot !=
            startup.assetRoot ||

            draft.startScene !=
            startup.startScene;
    }
}

ProjectSettingsPanel::
ProjectSettingsPanel(
    const ProjectConfig& initialConfig,
    const std::wstring& configPath)
    :
    m_startupConfig(initialConfig),
    m_savedConfig(initialConfig),
    m_draftConfig(initialConfig),
    m_configPath(configPath)
{
    SyncTextBuffers();
}

void ProjectSettingsPanel::Draw()
{
    ImGui::Begin(
        "Project Settings"
    );

    if (IsDirty())
    {
        ImGui::Text(
            "Status: Modified"
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Status: Saved"
        );
    }

    if (IsRestartRequired())
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "| Restart Required"
        );
    }

    ImGui::Separator();

    //
    // Window
    //

    ImGui::SeparatorText(
        "Window"
    );

    if (ImGui::InputText(
        "Title",
        m_windowTitleBuffer.data(),
        m_windowTitleBuffer.size()))
    {
        if (!ApplyBuffer(
            m_windowTitleBuffer,
            m_draftConfig.
            engine.windowTitle))
        {
            m_textEncodingError =
                true;
        }
        else
        {
            m_textEncodingError =
                false;
        }
    }

    ImGui::TextDisabled(
        "Title is applied immediately."
    );

    ImGui::InputInt(
        "Width",
        &m_draftConfig.
        engine.windowWidth
    );

    ImGui::InputInt(
        "Height",
        &m_draftConfig.
        engine.windowHeight
    );

    const char* windowModes[] =
    {
        "Windowed",
        "BorderlessFullscreen"
    };

    int windowModeIndex =
        m_draftConfig.engine.windowMode ==
        WindowMode::
        BorderlessFullscreen
        ? 1
        : 0;

    if (ImGui::Combo(
        "Window Mode",
        &windowModeIndex,
        windowModes,
        IM_ARRAYSIZE(windowModes)))
    {
        m_draftConfig.engine.windowMode =
            windowModeIndex == 0
            ? WindowMode::Windowed
            : WindowMode::
            BorderlessFullscreen;
    }

    ImGui::Checkbox(
        "Pause When Unfocused",
        &m_draftConfig.
        engine.pauseWhenUnfocused
    );

    ImGui::TextDisabled(
        "Size and Window Mode require restart."
    );

    //
    // Rendering
    //

    ImGui::SeparatorText(
        "Rendering"
    );

    ImGui::Checkbox(
        "VSync",
        &m_draftConfig.engine.vsync
    );

    int targetFPS =
        static_cast<int>(
            std::min<std::uint32_t>(
                m_draftConfig.
                engine.targetFPS,
                static_cast<std::uint32_t>(
                    std::numeric_limits<int>::
                    max())
            )
            );

    if (ImGui::InputInt(
        "Target FPS",
        &targetFPS))
    {
        targetFPS =
            std::max(
                targetFPS,
                0
            );

        m_draftConfig.engine.targetFPS =
            static_cast<std::uint32_t>(
                targetFPS
                );
    }

    ImGui::TextDisabled(
        "0 = Unlimited"
    );

    //
    // Fixed Update
    //

    ImGui::SeparatorText(
        "Fixed Update"
    );

    ImGui::InputFloat(
        "Fixed Update Hz",
        &m_draftConfig.
        engine.fixedUpdateHz,
        1.0f,
        10.0f,
        "%.2f"
    );

    int maxFixedSteps =
        static_cast<int>(
            std::min<std::uint32_t>(
                m_draftConfig.
                engine.maxFixedSteps,
                static_cast<std::uint32_t>(
                    std::numeric_limits<int>::
                    max())
            )
            );

    if (ImGui::InputInt(
        "Max Fixed Steps",
        &maxFixedSteps))
    {
        maxFixedSteps =
            std::max(
                maxFixedSteps,
                0
            );

        m_draftConfig.
            engine.maxFixedSteps =
            static_cast<std::uint32_t>(
                maxFixedSteps
                );
    }

    ImGui::InputFloat(
        "Max Delta Time",
        &m_draftConfig.
        engine.maxDeltaTime,
        0.01f,
        0.05f,
        "%.3f"
    );

    ImGui::TextDisabled(
        "Fixed-update settings require restart."
    );

    //
    // Debug
    //

    ImGui::SeparatorText(
        "Debug"
    );

    ImGui::Checkbox(
        "Initial Debug Collider",
        &m_draftConfig.
        engine.showDebugCollider
    );

    ImGui::Checkbox(
        "Show Runtime Stats",
        &m_draftConfig.
        engine.showRuntimeStats
    );

    ImGui::TextDisabled(
        "Initial Debug Collider is a startup default. "
        "F1 controls the current runtime state."
    );

    //
    // Project
    //

    ImGui::SeparatorText(
        "Project"
    );

    if (ImGui::InputText(
        "Asset Root",
        m_assetRootBuffer.data(),
        m_assetRootBuffer.size()))
    {
        if (!ApplyBuffer(
            m_assetRootBuffer,
            m_draftConfig.assetRoot))
        {
            m_textEncodingError =
                true;
        }
        else
        {
            m_textEncodingError =
                false;
        }
    }

    if (ImGui::InputText(
        "Start Scene",
        m_startSceneBuffer.data(),
        m_startSceneBuffer.size()))
    {
        if (!ApplyBuffer(
            m_startSceneBuffer,
            m_draftConfig.startScene))
        {
            m_textEncodingError =
                true;
        }
        else
        {
            m_textEncodingError =
                false;
        }
    }

    ImGui::TextDisabled(
        "Asset Root and Start Scene "
        "are startup project settings."
    );

    if (m_textEncodingError)
    {
        ImGui::Separator();

        ImGui::TextDisabled(
            "One or more text fields contain "
            "invalid UTF-8 data."
        );
    }

    //
// Save / Revert
//

    ImGui::Separator();

    const bool isDirty =
        IsDirty();

    if (!isDirty)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button(
        "Save"))
    {
        Save();
    }

    ImGui::SameLine();

    if (ImGui::Button(
        "Revert"))
    {
        Revert();
    }

    if (!isDirty)
    {
        ImGui::EndDisabled();
    }

    if (m_lastSaveSucceeded)
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "Saved."
        );
    }
    else if (m_lastSaveFailed)
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "Save failed."
        );
    }

    ImGui::End();
}

const ProjectConfig&
ProjectSettingsPanel::
GetDraftConfig() const
{
    return m_draftConfig;
}

bool ProjectSettingsPanel::
IsDirty() const
{
    return !SameProjectConfig(
        m_draftConfig,
        m_savedConfig
    );
}

bool ProjectSettingsPanel::
IsRestartRequired() const
{
    return RequiresRestart(
        m_draftConfig,
        m_startupConfig
    );
}

void ProjectSettingsPanel::
SyncTextBuffers()
{
    m_textEncodingError = false;

    if (!CopyWideToBuffer(
        m_draftConfig.engine.windowTitle,
        m_windowTitleBuffer))
    {
        m_textEncodingError = true;
    }

    if (!CopyWideToBuffer(
        m_draftConfig.assetRoot,
        m_assetRootBuffer))
    {
        m_textEncodingError = true;
    }

    if (!CopyWideToBuffer(
        m_draftConfig.startScene,
        m_startSceneBuffer))
    {
        m_textEncodingError = true;
    }
}

bool ProjectSettingsPanel::Save()
{
    m_lastSaveSucceeded = false;
    m_lastSaveFailed = false;

    if (m_textEncodingError)
    {
        m_lastSaveFailed = true;
        return false;
    }

    if (!ProjectConfigSerializer::Save(
        m_draftConfig,
        m_configPath))
    {
        m_lastSaveFailed = true;
        return false;
    }

    m_savedConfig =
        m_draftConfig;

    m_lastSaveSucceeded =
        true;

    return true;
}

void ProjectSettingsPanel::Revert()
{
    m_draftConfig =
        m_savedConfig;

    SyncTextBuffers();

    m_lastSaveSucceeded =
        false;

    m_lastSaveFailed =
        false;
}