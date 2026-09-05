#include "AnimationClipEditorPanel.h"

#include "Engine/Serialization/AnimationClipSerializer.h"
#include "Engine/Animation/AnimationClip.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"

#include <imgui.h>

#include <Windows.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <utility>

AnimationClipEditorPanel::
AnimationClipEditorPanel() = default;

AnimationClipEditorPanel::
~AnimationClipEditorPanel() = default;

bool AnimationClipEditorPanel::Open(
    const std::wstring& path)
{
    m_lastOpenFailed = false;
    m_openBlockedByDirty = false;

    if (m_isOpen &&
        path == m_documentPath)
    {
        return true;
    }

    if (m_isOpen &&
        IsDirty())
    {
        m_openBlockedByDirty =
            true;

        return false;
    }

    auto data =
        AnimationClipSerializer::Load(
            path
        );

    if (!data)
    {
        m_lastOpenFailed = true;

        return false;
    }

    m_savedData =
        *data;

    m_draftData =
        *data;

    m_documentPath =
        path;

    SyncTexturePathBuffer();

    m_isOpen = true;

    m_runtimeInSync = true;

    m_lastSaveSucceeded = false;
    m_lastSaveFailed = false;
    m_lastApplyFailed = false;

    return true;
}

void AnimationClipEditorPanel::Close()
{
    m_savedData =
        AnimationClipData{};

    m_draftData =
        AnimationClipData{};

    m_documentPath.clear();

    m_texturePathBuffer.fill(
        '\0'
    );

    m_isOpen = false;
    m_runtimeInSync = true;

    m_lastOpenFailed = false;
    m_openBlockedByDirty = false;

    m_lastSaveSucceeded = false;
    m_lastSaveFailed = false;

    m_lastApplyFailed = false;
    m_textEncodingError = false;
}

void AnimationClipEditorPanel::
DrawContents(
    ResourceManager& resources)
{
    if (!m_isOpen)
    {
        if (m_lastOpenFailed)
        {
            ImGui::TextDisabled(
                "Failed to open "
                "AnimationClip.");
        }
        else
        {
            ImGui::TextDisabled(
                "No AnimationClip open.");
        }

        return;
    }

    const bool dirty =
        IsDirty();

    const bool valid =
        AnimationClipSerializer::
        Validate(
            m_draftData
        );

    const std::string path =
        ToUtf8(
            m_documentPath
        );

    ImGui::TextWrapped(
        "%s",
        path.c_str()
    );

    if (dirty)
    {
        ImGui::Text(
            "Status: Modified");
    }
    else
    {
        ImGui::TextDisabled(
            "Status: Saved");
    }

    ImGui::SameLine();

    if (m_runtimeInSync)
    {
        ImGui::TextDisabled(
            "| Runtime: Synced");
    }
    else
    {
        ImGui::Text(
            "| Runtime: Out of Sync");
    }

    if (m_openBlockedByDirty)
    {
        ImGui::Text(
            "Save or Revert the current "
            "document before opening "
            "another AnimationClip.");
    }

    //
    // Save
    //

    ImGui::BeginDisabled(
        !dirty ||
        !valid
    );

    if (ImGui::Button(
        "Save"))
    {
        Save(
            resources
        );
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    //
    // Revert
    //

    ImGui::BeginDisabled(
        !dirty
    );

    if (ImGui::Button(
        "Revert"))
    {
        Revert(
            resources
        );
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    //
    // Close
    //

    ImGui::BeginDisabled(
        dirty
    );


    const bool closeRequested =
        ImGui::Button(
            "Close"
        );

    ImGui::EndDisabled();

    if (closeRequested)
    {
        Close();

        return;
    }

    if (m_lastSaveSucceeded)
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "Saved.");
    }
    else if (m_lastSaveFailed)
    {
        ImGui::SameLine();

        ImGui::Text(
            "Save failed.");
    }

    if (!valid)
    {
        ImGui::Text(
            "Draft is invalid. "
            "Runtime keeps the last "
            "valid definition.");
    }
    else if (m_lastApplyFailed)
    {
        ImGui::Text(
            "Live apply failed. "
            "Check the texture path.");
    }

    ImGui::SeparatorText(
        "Clip");

    bool applyRequested =
        false;

    //
    // Looping
    //

    if (ImGui::Checkbox(
        "Looping",
        &m_draftData.looping))
    {
        applyRequested =
            true;
    }

    ImGui::Text(
        "Frames: %zu",
        m_draftData.frames.size());

    ImGui::Text(
        "Total Duration: %.3f s",
        CalculateTotalDuration(
            m_draftData
        ));

    //
    // Texture
    //

    ImGui::SeparatorText(
        "Texture");

    if (ImGui::InputText(
        "Texture Path",
        m_texturePathBuffer.data(),
        m_texturePathBuffer.size()))
    {
        const std::string utf8
        {
            m_texturePathBuffer.data()
        };

        if (utf8.empty())
        {
            m_draftData.
                texturePath.clear();

            m_textEncodingError =
                false;
        }
        else
        {
            const std::wstring wide =
                Utf8ToWide(
                    utf8
                );

            if (!wide.empty())
            {
                m_draftData.
                    texturePath =
                    wide;

                m_textEncodingError =
                    false;
            }
            else
            {
                m_textEncodingError =
                    true;
            }
        }
    }

    if (ImGui::
        IsItemDeactivatedAfterEdit())
    {
        applyRequested =
            true;
    }

    if (m_textEncodingError)
    {
        ImGui::Text(
            "Texture path contains "
            "invalid UTF-8.");
    }

    //
    // Frames
    //

    ImGui::SeparatorText(
        "Frames");

    if (ImGui::Button(
        "Add Frame"))
    {
        AnimationClipFrameData frame{};

        if (!m_draftData.frames.empty())
        {
            frame =
                m_draftData.
                frames.back();
        }

        m_draftData.frames.
            emplace_back(
                frame
            );

        applyRequested =
            true;
    }

    int removeIndex = -1;
    int moveUpIndex = -1;
    int moveDownIndex = -1;

    for (std::size_t i = 0;
        i < m_draftData.frames.size();
        ++i)
    {
        AnimationClipFrameData& frame =
            m_draftData.frames[i];

        ImGui::PushID(
            static_cast<int>(i)
        );

        ImGui::Separator();

        ImGui::Text(
            "Frame %zu",
            i);

        float uv[4]
        {
            frame.uv.u0,
            frame.uv.v0,
            frame.uv.u1,
            frame.uv.v1
        };

        if (ImGui::DragFloat4(
            "UV",
            uv,
            0.001f,
            0.0f,
            1.0f,
            "%.6f",
            ImGuiSliderFlags_AlwaysClamp))
        {
            frame.uv =
            {
                uv[0],
                uv[1],
                uv[2],
                uv[3]
            };

            applyRequested =
                true;
        }

        if (ImGui::DragFloat(
            "Duration",
            &frame.duration,
            0.001f,
            0.001f,
            60.0f,
            "%.3f",
            ImGuiSliderFlags_AlwaysClamp))
        {
            applyRequested =
                true;
        }

        ImGui::BeginDisabled(
            i == 0);

        if (ImGui::Button(
            "Up"))
        {
            moveUpIndex =
                static_cast<int>(i);
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(
            i + 1 >=
            m_draftData.frames.size());

        if (ImGui::Button(
            "Down"))
        {
            moveDownIndex =
                static_cast<int>(i);
        }

        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(
            m_draftData.frames.size()
            <= 1);

        if (ImGui::Button(
            "Remove"))
        {
            removeIndex =
                static_cast<int>(i);
        }

        ImGui::EndDisabled();

        ImGui::PopID();
    }

    if (moveUpIndex > 0)
    {
        const std::size_t index =
            static_cast<std::size_t>(
                moveUpIndex);

        std::swap(
            m_draftData.frames[index],
            m_draftData.frames[
                index - 1]);

        applyRequested =
            true;
    }

    if (moveDownIndex >= 0)
    {
        const std::size_t index =
            static_cast<std::size_t>(
                moveDownIndex);

        if (index + 1 <
            m_draftData.frames.size())
        {
            std::swap(
                m_draftData.frames[index],
                m_draftData.frames[
                    index + 1]);

            applyRequested =
                true;
        }
    }

    if (removeIndex >= 0 &&
        m_draftData.frames.size() > 1)
    {
        m_draftData.frames.erase(
            m_draftData.frames.begin() +
            removeIndex
        );

        applyRequested =
            true;
    }

    //
    // Automatic live apply.
    //

    if (applyRequested)
    {
        m_lastSaveSucceeded =
            false;

        if (AnimationClipSerializer::
            Validate(
                m_draftData) &&
            !m_textEncodingError)
        {
            ApplyDraftToRuntime(
                resources
            );
        }
        else
        {
            m_runtimeInSync =
                false;

            m_lastApplyFailed =
                false;
        }
    }
}

const std::wstring&
AnimationClipEditorPanel::
GetDocumentPath() const noexcept
{
    return m_documentPath;
}

bool AnimationClipEditorPanel::
IsOpen() const noexcept
{
    return m_isOpen;
}

bool AnimationClipEditorPanel::
IsDirty() const noexcept
{
    if (!m_isOpen)
    {
        return false;
    }

    return
        !AreEqual(
            m_savedData,
            m_draftData
        );
}

bool AnimationClipEditorPanel::Save(
    ResourceManager& resources)
{
    m_lastSaveSucceeded = false;
    m_lastSaveFailed = false;

    if (!AnimationClipSerializer::
        Validate(
            m_draftData))
    {
        m_lastSaveFailed = true;

        return false;
    }

    //
    // Do not persist a definition that
    // cannot be applied to runtime.
    //

    if (!ApplyDraftToRuntime(
        resources))
    {
        m_lastSaveFailed = true;

        return false;
    }

    if (!AnimationClipSerializer::Save(
        m_draftData,
        m_documentPath))
    {
        m_lastSaveFailed = true;

        return false;
    }

    m_savedData =
        m_draftData;

    m_lastSaveSucceeded = true;

    return true;
}

void AnimationClipEditorPanel::Revert(
    ResourceManager& resources)
{
    m_draftData =
        m_savedData;

    SyncTexturePathBuffer();

    ApplyDraftToRuntime(
        resources
    );

    m_lastSaveSucceeded = false;
    m_lastSaveFailed = false;
    m_textEncodingError = false;
}

bool AnimationClipEditorPanel::
ApplyDataToRuntime(
    const AnimationClipData& data,
    ResourceManager& resources)
{
    if (!AnimationClipSerializer::
        Validate(
            data))
    {
        return false;
    }

    //
    // Resolve all external resources
    // before mutating the cached clip.
    //

    Texture* texture =
        resources.LoadTexture(
            data.texturePath
        );

    if (!texture)
    {
        return false;
    }

    AnimationClip* runtimeClip =
        resources.LoadAnimationClip(
            m_documentPath
        );

    if (!runtimeClip)
    {
        return false;
    }

    std::vector<AnimationFrame>
        runtimeFrames;

    runtimeFrames.reserve(
        data.frames.size()
    );

    for (const
        AnimationClipFrameData& frame :
        data.frames)
    {
        AnimationFrame
            runtimeFrame{};

        runtimeFrame.uv =
            frame.uv;

        runtimeFrame.duration =
            frame.duration;

        runtimeFrames.emplace_back(
            runtimeFrame
        );
    }

    return
        runtimeClip->
        ReplaceContents(
            texture,
            data.looping,
            std::move(
                runtimeFrames
            )
        );
}

bool AnimationClipEditorPanel::
ApplyDraftToRuntime(
    ResourceManager& resources)
{
    const bool result =
        ApplyDataToRuntime(
            m_draftData,
            resources
        );

    m_runtimeInSync =
        result;

    m_lastApplyFailed =
        !result;

    return result;
}

void AnimationClipEditorPanel::
SyncTexturePathBuffer()
{
    m_texturePathBuffer.fill(
        '\0'
    );

    const std::string utf8 =
        ToUtf8(
            m_draftData.texturePath
        );

    if (utf8.size() >=
        m_texturePathBuffer.size())
    {
        m_textEncodingError = true;

        return;
    }

    std::copy(
        utf8.begin(),
        utf8.end(),
        m_texturePathBuffer.begin()
    );

    m_textEncodingError = false;
}

bool AnimationClipEditorPanel::
AreEqual(
    const AnimationClipData& lhs,
    const AnimationClipData& rhs)
    noexcept
{
    if (lhs.texturePath !=
        rhs.texturePath)
    {
        return false;
    }

    if (lhs.looping !=
        rhs.looping)
    {
        return false;
    }

    if (lhs.frames.size() !=
        rhs.frames.size())
    {
        return false;
    }

    for (std::size_t i = 0;
        i < lhs.frames.size();
        ++i)
    {
        const auto& a =
            lhs.frames[i];

        const auto& b =
            rhs.frames[i];

        if (a.uv.u0 != b.uv.u0 ||
            a.uv.v0 != b.uv.v0 ||
            a.uv.u1 != b.uv.u1 ||
            a.uv.v1 != b.uv.v1 ||
            a.duration != b.duration)
        {
            return false;
        }
    }

    return true;
}

std::string
AnimationClipEditorPanel::ToUtf8(
    const std::wstring& value)
{
    if (value.empty())
    {
        return {};
    }

    if (value.size() >
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()))
    {
        return {};
    }

    const int sourceLength =
        static_cast<int>(
            value.size());

    const int requiredSize =
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            nullptr,
            0,
            nullptr,
            nullptr);

    if (requiredSize <= 0)
    {
        return {};
    }

    std::string result(
        static_cast<std::size_t>(
            requiredSize),
        '\0');

    const int convertedSize =
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            result.data(),
            requiredSize,
            nullptr,
            nullptr);

    if (convertedSize !=
        requiredSize)
    {
        return {};
    }

    return result;
}

std::wstring
AnimationClipEditorPanel::
Utf8ToWide(
    const std::string& value)
{
    if (value.empty())
    {
        return {};
    }

    if (value.size() >
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()))
    {
        return {};
    }

    const int sourceLength =
        static_cast<int>(
            value.size()
            );

    const int requiredSize =
        ::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            nullptr,
            0
        );

    if (requiredSize <= 0)
    {
        return {};
    }

    std::wstring result(
        static_cast<std::size_t>(
            requiredSize),
        L'\0'
    );

    const int convertedSize =
        ::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            result.data(),
            requiredSize
        );

    if (convertedSize !=
        requiredSize)
    {
        return {};
    }

    return result;
}

float
AnimationClipEditorPanel::
CalculateTotalDuration(
    const AnimationClipData& data)
    noexcept
{
    float total = 0.0f;

    for (const
        AnimationClipFrameData& frame :
        data.frames)
    {
        total +=
            frame.duration;
    }

    return total;
}