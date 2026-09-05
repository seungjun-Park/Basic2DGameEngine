#pragma once

#include "Engine/Animation/AnimationClipData.h"

#include <string>

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

    void DrawContents();

    [[nodiscard]]
    const std::wstring&
        GetDocumentPath() const noexcept;

    [[nodiscard]]
    bool IsOpen() const noexcept;

private:
    static std::string ToUtf8(
        const std::wstring& value);

    static float CalculateTotalDuration(
        const AnimationClipData& data)
        noexcept;

private:
    AnimationClipData m_data{};

    std::wstring m_documentPath;

    bool m_isOpen = false;
    bool m_lastOpenFailed = false;
};