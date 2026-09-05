#pragma once

#include "AssetDatabase.h"
#include "Engine/GUI/ProjectSettingsPanel.h"

#include <cstdint>
#include <memory>
#include <string>

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
        const std::wstring& assetRoot);

    void Shutdown();

    void Draw();
    void EnterPlayMode();
    void StopPlayMode();

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
    bool IsEditMode() const noexcept;

    [[nodiscard]]
    bool IsPlayMode() const noexcept;

    [[nodiscard]]
    bool IsInitialized() const noexcept;

private:
    void DrawModeControls();

private:
    std::unique_ptr<ProjectSettingsPanel>
        m_projectSettingsPanel;

    AssetDatabase m_assetDatabase;

    Mode m_mode = Mode::Edit;
    bool m_initialized = false;
};