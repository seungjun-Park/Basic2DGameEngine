#include "EditorSystem.h"

#include "Engine/GUI/ProjectSettingsPanel.h"

#include <utility>

EditorSystem::EditorSystem() = default;

EditorSystem::~EditorSystem()
{
    Shutdown();
}

bool EditorSystem::Initialize(
    std::unique_ptr<ProjectSettingsPanel>
    projectSettingsPanel)
{
    if (m_initialized)
    {
        return false;
    }

    if (!projectSettingsPanel)
    {
        return false;
    }

    m_projectSettingsPanel =
        std::move(projectSettingsPanel);

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

    m_mode = Mode::Edit;
    m_initialized = false;
}

void EditorSystem::Draw()
{
    if (!m_initialized)
    {
        return;
    }

    if (m_projectSettingsPanel)
    {
        m_projectSettingsPanel->Draw();
    }
}

EditorSystem::Mode
EditorSystem::GetMode() const noexcept
{
    return m_mode;
}

bool EditorSystem::IsEditMode() const noexcept
{
    return m_mode == Mode::Edit;
}

bool EditorSystem::IsPlayMode() const noexcept
{
    return m_mode == Mode::Play;
}

bool EditorSystem::IsInitialized() const noexcept
{
    return m_initialized;
}