#pragma once

#include <cstdint>
#include <memory>

class ProjectSettingsPanel;

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
        projectSettingsPanel);

    void Shutdown();

    void Draw();

    [[nodiscard]]
    Mode GetMode() const noexcept;

    [[nodiscard]]
    bool IsEditMode() const noexcept;

    [[nodiscard]]
    bool IsPlayMode() const noexcept;

    [[nodiscard]]
    bool IsInitialized() const noexcept;

private:
    std::unique_ptr<ProjectSettingsPanel>
        m_projectSettingsPanel;

    Mode m_mode = Mode::Edit;

    bool m_initialized = false;
};