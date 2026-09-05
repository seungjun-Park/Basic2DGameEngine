#pragma once

#include "Engine/Core/ProjectConfig.h"

#include <array>
#include <string>

class ProjectSettingsPanel
{
public:
    ProjectSettingsPanel(
        const ProjectConfig& initialConfig,
        const std::wstring& configPath
    );

    void Draw();

    const ProjectConfig&
        GetDraftConfig() const;

    bool IsDirty() const;
    bool IsRestartRequired() const;

private:
    void SyncTextBuffers();

    bool Save();
    void Revert();

private:
    ProjectConfig m_startupConfig{};
    ProjectConfig m_savedConfig{};
    ProjectConfig m_draftConfig{};

    std::wstring m_configPath;

    std::array<char, 2048>
        m_windowTitleBuffer{};

    std::array<char, 2048>
        m_assetRootBuffer{};

    std::array<char, 2048>
        m_startSceneBuffer{};

    bool m_lastSaveSucceeded = false;
    bool m_lastSaveFailed = false;
    bool m_textEncodingError = false;
};