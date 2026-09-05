#include "EditorSystem.h"

#include "Engine/Core/ProjectConfig.h"
#include "Engine/Core/Engine.h"
#include "Engine/GUI/EngineGui.h"
#include "Engine/Scene/Scene.h"

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

    m_assetBrowserPanel.Reset();

    m_animationClipEditorPanel.Close();

    m_selectedAssetPath.clear();

    m_mode = Mode::Edit;

    m_selectAnimationTabRequested =
        false;
    m_initialized = true;

    return true;
}

void EditorSystem::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    m_selectedAssetPath.clear();

    m_animationClipEditorPanel.Close();

    m_assetBrowserPanel.Reset();

    m_projectSettingsPanel.reset();

    m_assetDatabase.Shutdown();

    m_mode = Mode::Edit;

    m_selectAnimationTabRequested =
        false;

    m_initialized = false;
}

void EditorSystem::Draw(
    Engine& engine)
{
    if (!m_initialized)
    {
        return;
    }

    ImGui::SetNextWindowSize(
        ImVec2(
            720.0f,
            620.0f),
        ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(
        "Editor"))
    {
        ImGui::End();
        return;
    }

    DrawToolbar();

    ImGui::Separator();

    DrawWorkspaceTabs(
        engine);

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

const std::wstring&
EditorSystem::
GetSelectedAssetPath()
const noexcept
{
    return m_selectedAssetPath;
}

bool EditorSystem::
HasSelectedAsset()
const noexcept
{
    return
        !m_selectedAssetPath.empty();
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

void EditorSystem::
DrawWorkspaceTabs(
    Engine& engine)
{
    if (!ImGui::BeginTabBar(
        "##EditorWorkspaceTabs"))
    {
        return;
    }

    //
    // Project Settings
    //

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

    //
    // Assets
    //

    if (ImGui::BeginTabItem(
        "Assets"))
    {
        const bool openRequested =
            m_assetBrowserPanel.
            DrawContents(
                m_assetDatabase,
                m_selectedAssetPath);

        if (openRequested &&
            !m_selectedAssetPath.empty())
        {
            const AssetRecord* asset =
                m_assetDatabase.FindAsset(
                    m_selectedAssetPath);

            if (asset &&
                asset->type ==
                AssetType::AnimationClip)
            {
                m_animationClipEditorPanel.
                    Open(
                        asset->path);

                m_selectAnimationTabRequested =
                    true;
            }
        }

        ImGui::EndTabItem();
    }

    //
    // Animation
    //

    ImGuiTabItemFlags
        animationTabFlags =
        ImGuiTabItemFlags_None;

    if (m_selectAnimationTabRequested)
    {
        animationTabFlags |=
            ImGuiTabItemFlags_SetSelected;
    }

    if (ImGui::BeginTabItem(
        "Animation",
        nullptr,
        animationTabFlags))
    {
        m_animationClipEditorPanel.
            DrawContents();

        ImGui::EndTabItem();
    }

    m_selectAnimationTabRequested =
        false;

    //
    // Audio
    //

    if (ImGui::BeginTabItem(
        "Audio"))
    {
        EngineGui::
            DrawAudioSettingsContents(
                engine.GetAudioSystem());

        ImGui::EndTabItem();
    }

    //
    // TileMap
    //

    if (ImGui::BeginTabItem(
        "TileMap"))
    {
        Scene* scene =
            engine.GetScene();

        if (scene)
        {
            scene->
                DrawGuiContents();
        }
        else
        {
            ImGui::TextDisabled(
                "No active scene.");
        }

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}