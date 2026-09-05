#pragma once

#include "Engine/GUI/ProjectSettingsPanel.h"

#include "Engine/Editor/AnimationClipEditorPanel.h"
#include "Engine/Editor/AssetBrowserPanel.h"
#include "Engine/Editor/AssetDatabase.h"
#include "Engine/Editor/InspectorPanel.h"
#include "Engine/Editor/SceneHierarchyPanel.h"
#include "Engine/Editor/TileMapEditorPanel.h"
#include "Engine/Editor/TilePalettePanel.h"
#include "Engine/Editor/TilesetEditorPanel.h"

#include "Engine/Scene/EntityHandle.h"

#include <cstdint>
#include <memory>
#include <string>

class Engine;
class ProjectConfig;

class EditorSystem
{
public:
    enum class Mode : std::uint8_t
    {
        Edit,
        Play
    };

    EditorSystem();
    ~EditorSystem();

    EditorSystem(
        const EditorSystem&) = delete;

    EditorSystem& operator=(
        const EditorSystem&) = delete;

    EditorSystem(
        EditorSystem&&) = delete;

    EditorSystem& operator=(
        EditorSystem&&) = delete;

    bool Initialize(
        std::unique_ptr<ProjectSettingsPanel>
        projectSettingsPanel,
        const std::wstring& assetRoot
    );

    void Shutdown();

    void Draw(
        Engine& engine
    );

    void EnterPlayMode();
    void StopPlayMode();

    void ClearEntitySelection() noexcept;

    [[nodiscard]]
    Mode GetMode() const noexcept;

    [[nodiscard]]
    const ProjectConfig&
        GetProjectSettingsDraftConfig() const;

    [[nodiscard]]
    AssetDatabase&
        GetAssetDatabase() noexcept;

    [[nodiscard]]
    const AssetDatabase&
        GetAssetDatabase() const noexcept;

    [[nodiscard]]
    const std::wstring&
        GetSelectedAssetPath() const noexcept;

    [[nodiscard]]
    bool HasSelectedAsset() const noexcept;

    [[nodiscard]]
    EntityHandle
        GetSelectedEntityHandle() const noexcept;

    [[nodiscard]]
    bool HasSelectedEntity() const noexcept;

    [[nodiscard]]
    bool IsEditMode() const noexcept;

    [[nodiscard]]
    bool IsPlayMode() const noexcept;

    [[nodiscard]]
    bool IsInitialized() const noexcept;

private:
    void DrawToolbar();

    void DrawSceneDocumentControls(
        Engine& engine
    );

    void DrawWorkspaceTabs(
        Engine& engine
    );

private:
    std::unique_ptr<ProjectSettingsPanel>
        m_projectSettingsPanel;

    AssetDatabase
        m_assetDatabase;

    AssetBrowserPanel
        m_assetBrowserPanel;

    AnimationClipEditorPanel
        m_animationClipEditorPanel;

    SceneHierarchyPanel
        m_sceneHierarchyPanel;

    InspectorPanel
        m_inspectorPanel;

    TilesetEditorPanel
        m_tilesetEditorPanel;

    TilePalettePanel
        m_tilePalettePanel;

    TileMapEditorPanel
        m_tileMapEditorPanel;

    std::wstring
        m_selectedAssetPath;

    std::wstring
        m_activeSceneDocumentPath;

    EntityHandle
        m_selectedEntityHandle{};

    Mode m_mode =
        Mode::Edit;

    bool m_sceneDirty =
        false;

    bool m_lastSceneSaveSucceeded =
        false;

    bool m_lastSceneSaveFailed =
        false;

    bool m_lastSceneLoadSucceeded =
        false;

    bool m_lastSceneLoadFailed =
        false;

    bool m_selectAnimationTabRequested =
        false;

    bool m_selectTilesetTabRequested =
        false;

    bool m_selectTileMapEditorTabRequested =
        false;

    bool m_initialized =
        false;
};