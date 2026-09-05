#include "EditorSystem.h"

#include "Engine/Core/ProjectConfig.h"

#include <imgui.h>

#include <utility>

EditorSystem::EditorSystem() = default;

EditorSystem::~EditorSystem()
{
    Shutdown();
}

bool EditorSystem::Initialize(
    std::unique_ptr<ProjectSettingsPanel>
    projectSettingsPanel,
    const std::wstring& assetRoot)
{
    if (m_initialized)
    {
        return false;
    }

    if (!projectSettingsPanel)
    {
        return false;
    }

    if (!m_assetDatabase.Initialize(
        assetRoot))
    {
        return false;
    }

    m_projectSettingsPanel =
        std::move(
            projectSettingsPanel
        );

    m_mode = Mode::Edit;
    m_initialized = true;

    return true;
}

void EditorSystem::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    m_projectSettingsPanel.reset();

    m_assetDatabase.Shutdown();

    m_mode = Mode::Edit;
    m_initialized = false;
}

void EditorSystem::Draw()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(
            720.0f,
            620.0f
        ),
        ImGuiCond_FirstUseEver
    );

    if (!ImGui::Begin(
        "Editor"))
    {
        ImGui::End();
        return;
    }

    DrawToolbar();

    ImGui::Separator();

    DrawWorkspaceTabs();

    ImGui::End();
}

void EditorSystem::EnterPlayMode()
{
    if (!m_initialized ||
        m_mode == Mode::Play)
    {
        return;
    }

    m_mode = Mode::Play;
}

void EditorSystem::StopPlayMode()
{
    if (!m_initialized ||
        m_mode == Mode::Edit)
    {
        return;
    }

    m_mode = Mode::Edit;
}

EditorSystem::Mode
EditorSystem::GetMode() const noexcept
{
    return m_mode;
}

const ProjectConfig&
EditorSystem::
GetProjectSettingsDraftConfig() const
{
    return
        m_projectSettingsPanel->
        GetDraftConfig();
}

AssetDatabase&
EditorSystem::GetAssetDatabase()
noexcept
{
    return m_assetDatabase;
}

const AssetDatabase&
EditorSystem::GetAssetDatabase()
const noexcept
{
    return m_assetDatabase;
}

bool EditorSystem::IsEditMode()
const noexcept
{
    return
        m_mode == Mode::Edit;
}

bool EditorSystem::IsPlayMode()
const noexcept
{
    return
        m_mode == Mode::Play;
}

bool EditorSystem::IsInitialized()
const noexcept
{
    return m_initialized;
}

void EditorSystem::DrawToolbar()
{
    if (m_mode == Mode::Edit)
    {
        if (ImGui::Button(
            "Play"))
        {
            EnterPlayMode();
        }

        ImGui::SameLine();

        ImGui::TextDisabled(
            "Edit Mode"
        );
    }
    else
    {
        if (ImGui::Button(
            "Stop"))
        {
            StopPlayMode();
        }

        ImGui::SameLine();

        ImGui::Text(
            "Play Mode"
        );
    }
}

void EditorSystem::DrawWorkspaceTabs()
{
    if (!ImGui::BeginTabBar(
        "##EditorWorkspaceTabs"))
    {
        return;
    }

    if (ImGui::BeginTabItem(
        "Project Settings"))
    {
        if (m_projectSettingsPanel)
        {
            m_projectSettingsPanel->
                DrawContents();
        }

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}