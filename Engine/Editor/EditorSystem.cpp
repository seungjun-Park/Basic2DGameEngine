#include "EditorSystem.h"

#include "Engine/Core/Engine.h"
#include "Engine/Core/ProjectConfig.h"

#include "Engine/GUI/EngineGui.h"

#include "Engine/Scene/IPlayModeSnapshotTarget.h"
#include "Engine/Scene/ISceneDocumentTarget.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Tile/ITileMapRuntimeTarget.h"

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

    m_activeSceneDocumentPath =
        m_projectSettingsPanel->
        GetDraftConfig().
        startScene;

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

    m_sceneDirty =
        false;

    m_lastSceneSaveSucceeded =
        false;

    m_lastSceneSaveFailed =
        false;

    m_lastSceneLoadSucceeded =
        false;

    m_lastSceneLoadFailed =
        false;

    m_lastPlaySnapshotCaptureFailed =
        false;

    m_lastPlaySnapshotRestoreFailed =
        false;

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

    m_activeSceneDocumentPath.clear();

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

    m_sceneDirty =
        false;

    m_lastSceneSaveSucceeded =
        false;

    m_lastSceneSaveFailed =
        false;

    m_lastSceneLoadSucceeded =
        false;

    m_lastSceneLoadFailed =
        false;

    m_lastPlaySnapshotCaptureFailed =
        false;

    m_lastPlaySnapshotRestoreFailed =
        false;

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

    DrawToolbar(
        engine
    );

    ImGui::Separator();

    DrawSceneDocumentControls(
        engine
    );

    ImGui::Separator();

    DrawWorkspaceTabs(
        engine
    );

    ImGui::End();
}

bool EditorSystem::EnterPlayMode(
    Engine& engine
)
{
    if (!m_initialized ||
        m_mode !=
        Mode::Edit)
    {
        return false;
    }

    m_lastPlaySnapshotCaptureFailed =
        false;

    m_lastPlaySnapshotRestoreFailed =
        false;

    Scene* scene =
        engine.GetScene();

    IPlayModeSnapshotTarget*
        snapshotTarget =
        dynamic_cast<
        IPlayModeSnapshotTarget*
        >(
            scene
            );

    if (!snapshotTarget)
    {
        m_lastPlaySnapshotCaptureFailed =
            true;

        return false;
    }

    if (snapshotTarget->
        HasPlaySnapshot())
    {
        m_lastPlaySnapshotCaptureFailed =
            true;

        return false;
    }

    if (!snapshotTarget->
        CapturePlaySnapshot())
    {
        m_lastPlaySnapshotCaptureFailed =
            true;

        return false;
    }

    m_mode =
        Mode::Play;

    return true;
}

bool EditorSystem::StopPlayMode(
    Engine& engine
)
{
    if (!m_initialized ||
        m_mode !=
        Mode::Play)
    {
        return false;
    }

    m_lastPlaySnapshotRestoreFailed =
        false;

    Scene* scene =
        engine.GetScene();

    IPlayModeSnapshotTarget*
        snapshotTarget =
        dynamic_cast<
        IPlayModeSnapshotTarget*
        >(
            scene
            );

    if (!snapshotTarget ||
        !snapshotTarget->
        HasPlaySnapshot())
    {
        m_lastPlaySnapshotRestoreFailed =
            true;

        return false;
    }

    if (!snapshotTarget->
        RestorePlaySnapshot())
    {
        //
        // Do not claim that we returned to
        // Edit Mode when restoration failed.
        //
        // Snapshot remains owned by Scene and
        // Stop may be attempted again.
        //

        m_lastPlaySnapshotRestoreFailed =
            true;

        ClearEntitySelection();

        return false;
    }

    m_mode =
        Mode::Edit;

    //
    // Restore recreates entities, therefore every
    // pre-Stop EntityHandle selection is stale.
    //

    ClearEntitySelection();

    return true;
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

void EditorSystem::DrawToolbar(
    Engine& engine
)
{
    if (m_mode ==
        Mode::Edit)
    {
        const bool playRequested =
            ImGui::Button(
                "Play"
            );

        if (playRequested)
        {
            EnterPlayMode(
                engine
            );
        }

        ImGui::SameLine();

        ImGui::TextDisabled(
            "Edit Mode"
        );
    }
    else
    {
        const bool stopRequested =
            ImGui::Button(
                "Stop"
            );

        if (stopRequested)
        {
            StopPlayMode(
                engine
            );
        }

        ImGui::SameLine();

        ImGui::Text(
            "Play Mode"
        );
    }

    if (m_lastPlaySnapshotCaptureFailed)
    {
        ImGui::TextDisabled(
            "Play failed: the Edit Mode "
            "snapshot could not be captured."
        );
    }

    if (m_lastPlaySnapshotRestoreFailed)
    {
        ImGui::TextDisabled(
            "Stop failed: the Play snapshot "
            "could not be restored."
        );
    }
}

void EditorSystem::
DrawSceneDocumentControls(
    Engine& engine
)
{
    Scene* scene =
        engine.GetScene();

    ISceneDocumentTarget*
        sceneDocumentTarget =
        dynamic_cast<
        ISceneDocumentTarget*
        >(
            scene
            );

    const AssetRecord*
        selectedAsset =
        nullptr;

    if (!m_selectedAssetPath.empty())
    {
        selectedAsset =
            m_assetDatabase.FindAsset(
                m_selectedAssetPath
            );
    }

    const bool selectedSceneAvailable =
        selectedAsset &&
        selectedAsset->type ==
        AssetType::Scene;

    const bool editMode =
        IsEditMode();

    const bool saveAvailable =
        editMode &&
        sceneDocumentTarget &&
        m_sceneDirty &&
        !m_activeSceneDocumentPath.empty();

    const bool revertAvailable =
        editMode &&
        sceneDocumentTarget &&
        m_sceneDirty &&
        !m_activeSceneDocumentPath.empty();

    const bool loadSelectedAvailable =
        editMode &&
        sceneDocumentTarget &&
        selectedSceneAvailable &&
        !m_sceneDirty;

    if (m_sceneDirty)
    {
        ImGui::Text(
            "Scene: Modified"
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Scene: Saved"
        );
    }

    if (m_tileMapEditorPanel.IsDirty())
    {
        ImGui::SameLine();

        ImGui::Text(
            "| TileMap: Modified"
        );
    }
    else if (
        m_tileMapEditorPanel.IsOpen())
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "| TileMap: Saved"
        );
    }

    ImGui::BeginDisabled(
        !saveAvailable
    );

    const bool saveRequested =
        ImGui::Button(
            "Save Scene"
        );

    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(
        !revertAvailable
    );

    const bool revertRequested =
        ImGui::Button(
            "Revert Scene"
        );

    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(
        !loadSelectedAvailable
    );

    const bool loadSelectedRequested =
        ImGui::Button(
            "Load Selected Scene"
        );

    ImGui::EndDisabled();

    if (saveRequested &&
        sceneDocumentTarget)
    {
        m_lastSceneSaveSucceeded =
            false;

        m_lastSceneSaveFailed =
            false;

        if (sceneDocumentTarget->
            SaveSceneDocument(
                m_activeSceneDocumentPath
            ))
        {
            m_sceneDirty =
                false;

            m_lastSceneSaveSucceeded =
                true;
        }
        else
        {
            m_lastSceneSaveFailed =
                true;
        }
    }

    if (revertRequested &&
        sceneDocumentTarget)
    {
        m_lastSceneLoadSucceeded =
            false;

        m_lastSceneLoadFailed =
            false;

        if (sceneDocumentTarget->
            LoadSceneDocument(
                m_activeSceneDocumentPath
            ))
        {
            m_sceneDirty =
                false;

            ClearEntitySelection();

            m_lastSceneLoadSucceeded =
                true;
        }
        else
        {
            m_lastSceneLoadFailed =
                true;
        }
    }

    if (loadSelectedRequested &&
        sceneDocumentTarget &&
        selectedAsset)
    {
        m_lastSceneLoadSucceeded =
            false;

        m_lastSceneLoadFailed =
            false;

        if (sceneDocumentTarget->
            LoadSceneDocument(
                selectedAsset->path
            ))
        {
            m_activeSceneDocumentPath =
                selectedAsset->path;

            m_sceneDirty =
                false;

            ClearEntitySelection();

            m_tileMapEditorPanel.Close();

            m_tilePalettePanel.Close();

            m_lastSceneLoadSucceeded =
                true;
        }
        else
        {
            m_lastSceneLoadFailed =
                true;
        }
    }

    if (m_sceneDirty &&
        selectedSceneAvailable)
    {
        ImGui::TextDisabled(
            "Save or Revert the current Scene "
            "before loading another Scene."
        );
    }

    if (!sceneDocumentTarget)
    {
        ImGui::TextDisabled(
            "The active Scene does not support "
            "document persistence."
        );
    }

    if (m_lastSceneSaveSucceeded)
    {
        ImGui::TextDisabled(
            "Scene save succeeded."
        );
    }

    if (m_lastSceneSaveFailed)
    {
        ImGui::TextDisabled(
            "Scene save failed."
        );
    }

    if (m_lastSceneLoadSucceeded)
    {
        ImGui::TextDisabled(
            "Scene load/revert succeeded."
        );
    }

    if (m_lastSceneLoadFailed)
    {
        ImGui::TextDisabled(
            "Scene load/revert failed."
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
        Scene* scene =
            engine.GetScene();

        ITileMapRuntimeTarget*
            runtimeTarget =
            dynamic_cast<
            ITileMapRuntimeTarget*
            >(
                scene
                );

        m_tileMapEditorPanel.
            DrawContents(
                engine.GetResourceManager(),
                m_tilePalettePanel.
                GetSelectedTileId(),
                m_tilePalettePanel.
                GetTilesetPath(),
                IsEditMode(),
                runtimeTarget
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
            const bool sceneChanged =
                m_inspectorPanel.
                DrawContents(
                    *entity,
                    m_assetDatabase,
                    m_selectedAssetPath,
                    engine.GetResourceManager(),
                    IsEditMode()
                );

            if (sceneChanged)
            {
                m_sceneDirty =
                    true;

                m_lastSceneSaveSucceeded =
                    false;

                m_lastSceneSaveFailed =
                    false;

                m_lastSceneLoadSucceeded =
                    false;

                m_lastSceneLoadFailed =
                    false;
            }
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