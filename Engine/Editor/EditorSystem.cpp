#include "EditorSystem.h"

#include "Engine/Core/Engine.h"
#include "Engine/Core/ProjectConfig.h"

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
    const std::wstring& assetRoot
)
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
        assetRoot
    ))
    {
        return false;
    }

    m_projectSettingsPanel =
        std::move(
            projectSettingsPanel
        );

    m_assetBrowserPanel.Reset();

    m_animationClipEditorPanel.Close();

    m_tilesetEditorPanel.Close();

    m_tilePalettePanel.Close();

    m_tileMapEditorPanel.Close();

    m_selectedAssetPath.clear();

    m_selectedEntityHandle =
        EntityHandle{};

    m_mode =
        Mode::Edit;

    m_selectAnimationTabRequested =
        false;

    m_selectTilesetTabRequested =
        false;

    m_selectTileMapEditorTabRequested =
        false;

    m_initialized =
        true;

    return true;
}

void EditorSystem::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    m_selectedAssetPath.clear();

    m_selectedEntityHandle =
        EntityHandle{};

    m_animationClipEditorPanel.Close();

    m_tilesetEditorPanel.Close();

    m_tilePalettePanel.Close();

    m_tileMapEditorPanel.Close();

    m_assetBrowserPanel.Reset();

    m_projectSettingsPanel.reset();

    m_assetDatabase.Shutdown();

    m_mode =
        Mode::Edit;

    m_selectAnimationTabRequested =
        false;

    m_selectTilesetTabRequested =
        false;

    m_selectTileMapEditorTabRequested =
        false;

    m_initialized =
        false;
}

void EditorSystem::Draw(
    Engine& engine
)
{
    if (!m_initialized)
    {
        return;
    }

    Scene* scene =
        engine.GetScene();

    if (!scene)
    {
        ClearEntitySelection();
    }
    else if (
        m_selectedEntityHandle.IsValid() &&
        !scene->IsEntityAlive(
            m_selectedEntityHandle
        ))
    {
        ClearEntitySelection();
    }

    ImGui::SetNextWindowSize(
        ImVec2(
            720.0f,
            620.0f
        ),
        ImGuiCond_FirstUseEver
    );

    if (!ImGui::Begin(
        "Editor"
    ))
    {
        ImGui::End();

        return;
    }

    DrawToolbar();

    ImGui::Separator();

    DrawWorkspaceTabs(
        engine
    );

    ImGui::End();
}

void EditorSystem::EnterPlayMode()
{
    if (!m_initialized ||
        m_mode ==
        Mode::Play)
    {
        return;
    }

    m_mode =
        Mode::Play;
}

void EditorSystem::StopPlayMode()
{
    if (!m_initialized ||
        m_mode ==
        Mode::Edit)
    {
        return;
    }

    m_mode =
        Mode::Edit;
}

void EditorSystem::
ClearEntitySelection() noexcept
{
    m_selectedEntityHandle =
        EntityHandle{};
}

EditorSystem::Mode
EditorSystem::GetMode()
const noexcept
{
    return
        m_mode;
}

const ProjectConfig&
EditorSystem::
GetProjectSettingsDraftConfig()
const
{
    return
        m_projectSettingsPanel->
        GetDraftConfig();
}

AssetDatabase&
EditorSystem::
GetAssetDatabase() noexcept
{
    return
        m_assetDatabase;
}

const AssetDatabase&
EditorSystem::
GetAssetDatabase() const noexcept
{
    return
        m_assetDatabase;
}

const std::wstring&
EditorSystem::
GetSelectedAssetPath() const noexcept
{
    return
        m_selectedAssetPath;
}

bool EditorSystem::
HasSelectedAsset() const noexcept
{
    return
        !m_selectedAssetPath.empty();
}

EntityHandle
EditorSystem::
GetSelectedEntityHandle()
const noexcept
{
    return
        m_selectedEntityHandle;
}

bool EditorSystem::
HasSelectedEntity() const noexcept
{
    return
        m_selectedEntityHandle.
        IsValid();
}

bool EditorSystem::
IsEditMode() const noexcept
{
    return
        m_mode ==
        Mode::Edit;
}

bool EditorSystem::
IsPlayMode() const noexcept
{
    return
        m_mode ==
        Mode::Play;
}

bool EditorSystem::
IsInitialized() const noexcept
{
    return
        m_initialized;
}

void EditorSystem::DrawToolbar()
{
    if (m_mode ==
        Mode::Edit)
    {
        if (ImGui::Button(
            "Play"
        ))
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
            "Stop"
        ))
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
    Engine& engine
)
{
    if (!ImGui::BeginTabBar(
        "##EditorWorkspaceTabs"
    ))
    {
        return;
    }

    //
    // Project Settings
    //

    if (ImGui::BeginTabItem(
        "Project Settings"
    ))
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
        "Assets"
    ))
    {
        const bool openRequested =
            m_assetBrowserPanel.
            DrawContents(
                m_assetDatabase,
                m_selectedAssetPath
            );

        if (openRequested &&
            !m_selectedAssetPath.empty())
        {
            const AssetRecord* asset =
                m_assetDatabase.FindAsset(
                    m_selectedAssetPath
                );

            if (asset)
            {
                if (asset->type ==
                    AssetType::AnimationClip)
                {
                    m_animationClipEditorPanel.
                        Open(
                            asset->path
                        );

                    m_selectAnimationTabRequested =
                        true;
                }
                else if (
                    asset->type ==
                    AssetType::Texture)
                {
                    if (m_tilesetEditorPanel.
                        OpenTexture(
                            asset->path,
                            engine.GetResourceManager()
                        ))
                    {
                        m_selectTilesetTabRequested =
                            true;
                    }
                }
                else if (
                    asset->type ==
                    AssetType::Tileset)
                {
                    if (m_tilesetEditorPanel.
                        Open(
                            asset->path,
                            engine.GetResourceManager()
                        ))
                    {
                        m_selectTilesetTabRequested =
                            true;
                    }
                }
                else if (
                    asset->type ==
                    AssetType::TileMap)
                {
                    if (m_tileMapEditorPanel.
                        Open(
                            asset->path,
                            engine.GetResourceManager()
                        ))
                    {
                        //
                        // Ensure that the palette uses
                        // the exact Tileset referenced
                        // by this TileMap document.
                        //

                        m_tilePalettePanel.
                            OpenTileset(
                                m_tileMapEditorPanel.
                                GetTilesetPath(),
                                engine.GetResourceManager()
                            );

                        m_selectTileMapEditorTabRequested =
                            true;
                    }
                }
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
        animationTabFlags
    ))
    {
        m_animationClipEditorPanel.
            DrawContents(
                engine.GetResourceManager()
            );

        ImGui::EndTabItem();
    }

    m_selectAnimationTabRequested =
        false;

    //
    // Audio
    //

    if (ImGui::BeginTabItem(
        "Audio"
    ))
    {
        EngineGui::
            DrawAudioSettingsContents(
                engine.GetAudioSystem()
            );

        ImGui::EndTabItem();
    }

    //
    // Runtime TileMap
    //

    if (ImGui::BeginTabItem(
        "TileMap"
    ))
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
                "No active scene."
            );
        }

        ImGui::EndTabItem();
    }

    //
    // Tileset
    //

    ImGuiTabItemFlags
        tilesetTabFlags =
        ImGuiTabItemFlags_None;

    if (m_selectTilesetTabRequested)
    {
        tilesetTabFlags |=
            ImGuiTabItemFlags_SetSelected;
    }

    if (ImGui::BeginTabItem(
        "Tileset",
        nullptr,
        tilesetTabFlags
    ))
    {
        m_tilesetEditorPanel.
            DrawContents();

        ImGui::EndTabItem();
    }

    m_selectTilesetTabRequested =
        false;

    //
    // Tile Palette
    //

    if (ImGui::BeginTabItem(
        "Tile Palette"
    ))
    {
        m_tilePalettePanel.
            DrawContents(
                m_assetDatabase,
                m_selectedAssetPath,
                engine.GetResourceManager()
            );

        ImGui::EndTabItem();
    }

    //
    // TileMap Editor
    //

    ImGuiTabItemFlags
        tileMapEditorTabFlags =
        ImGuiTabItemFlags_None;

    if (m_selectTileMapEditorTabRequested)
    {
        tileMapEditorTabFlags |=
            ImGuiTabItemFlags_SetSelected;
    }

    if (ImGui::BeginTabItem(
        "TileMap Editor",
        nullptr,
        tileMapEditorTabFlags
    ))
    {
        m_tileMapEditorPanel.
            DrawContents(
                engine.GetResourceManager(),
                m_tilePalettePanel.
                GetSelectedTileId(),
                m_tilePalettePanel.
                GetTilesetPath(),
                IsEditMode()
            );

        ImGui::EndTabItem();
    }

    m_selectTileMapEditorTabRequested =
        false;

    //
    // Hierarchy
    //

    if (ImGui::BeginTabItem(
        "Hierarchy"
    ))
    {
        const SceneHierarchyPanel::
            SelectionResult selectionResult =
            m_sceneHierarchyPanel.
            DrawContents(
                engine.GetScene(),
                m_selectedEntityHandle
            );

        if (selectionResult.changed)
        {
            m_selectedEntityHandle =
                selectionResult.handle;
        }

        ImGui::EndTabItem();
    }

    //
    // Inspector
    //

    if (ImGui::BeginTabItem(
        "Inspector"
    ))
    {
        Scene* scene =
            engine.GetScene();

        Entity* entity =
            nullptr;

        if (scene &&
            m_selectedEntityHandle.IsValid())
        {
            entity =
                scene->ResolveEntity(
                    m_selectedEntityHandle
                );
        }

        if (entity)
        {
            m_inspectorPanel.
                DrawContents(
                    *entity,
                    m_assetDatabase,
                    m_selectedAssetPath,
                    engine.GetResourceManager(),
                    IsEditMode()
                );
        }
        else
        {
            ImGui::TextDisabled(
                "No entity selected."
            );
        }

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}