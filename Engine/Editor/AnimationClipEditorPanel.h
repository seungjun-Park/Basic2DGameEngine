#pragma once

#include "Engine/Animation/AnimationClipData.h"

#include <array>
#include <string>

class ResourceManager;

class AnimationClipEditorPanel
{
public:
    AnimationClipEditorPanel();
    ~AnimationClipEditorPanel();

    AnimationClipEditorPanel(
        const AnimationClipEditorPanel&) =
        delete;

    AnimationClipEditorPanel& operator=(
        const AnimationClipEditorPanel&) =
        delete;

    AnimationClipEditorPanel(
        AnimationClipEditorPanel&&) =
        delete;

    AnimationClipEditorPanel& operator=(
        AnimationClipEditorPanel&&) =
        delete;

    bool Open(
        const std::wstring& path);

    void Close();

    void DrawContents(
        ResourceManager& resources);

    [[nodiscard]]
    const std::wstring&
        GetDocumentPath() const noexcept;

    [[nodiscard]]
    bool IsOpen() const noexcept;

    [[nodiscard]]
    bool IsDirty() const noexcept;

private:
    bool Save(
        ResourceManager& resources);

    void Revert(
        ResourceManager& resources);

    bool ApplyDraftToRuntime(
        ResourceManager& resources);

    bool ApplyDataToRuntime(
        const AnimationClipData& data,
        ResourceManager& resources);

    void SyncTexturePathBuffer();

    static bool AreEqual(
        const AnimationClipData& lhs,
        const AnimationClipData& rhs)
        noexcept;

    static std::string ToUtf8(
        const std::wstring& value);

    static std::wstring Utf8ToWide(
        const std::string& value);

    static float CalculateTotalDuration(
        const AnimationClipData& data)
        noexcept;

private:
    AnimationClipData m_savedData{};
    AnimationClipData m_draftData{};

    std::wstring m_documentPath;

    std::array<char, 2048>
        m_texturePathBuffer{};

    bool m_isOpen = false;

    bool m_runtimeInSync = true;

    bool m_lastOpenFailed = false;
    bool m_openBlockedByDirty = false;

    bool m_lastSaveSucceeded = false;
    bool m_lastSaveFailed = false;

    bool m_lastApplyFailed = false;
    bool m_textEncodingError = false;
};