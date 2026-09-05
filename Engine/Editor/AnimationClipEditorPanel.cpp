#include "AnimationClipEditorPanel.h"

#include "Engine/Serialization/AnimationClipSerializer.h"

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
    auto data =
        AnimationClipSerializer::Load(
            path);

    if (!data)
    {
        m_data =
            AnimationClipData{};

        m_documentPath =
            path;

        m_isOpen = false;
        m_lastOpenFailed = true;

        return false;
    }

    m_data =
        std::move(
            *data);

    m_documentPath =
        path;

    m_isOpen = true;
    m_lastOpenFailed = false;

    return true;
}

void AnimationClipEditorPanel::Close()
{
    m_data =
        AnimationClipData{};

    m_documentPath.clear();

    m_isOpen = false;
    m_lastOpenFailed = false;
}

void AnimationClipEditorPanel::
DrawContents()
{
    if (!m_isOpen)
    {
        if (m_lastOpenFailed)
        {
            const std::string path =
                ToUtf8(
                    m_documentPath);

            ImGui::TextDisabled(
                "Failed to open "
                "AnimationClip.");

            if (!path.empty())
            {
                ImGui::TextWrapped(
                    "%s",
                    path.c_str());
            }
        }
        else
        {
            ImGui::TextDisabled(
                "No AnimationClip open.");

            ImGui::TextDisabled(
                "Double-click an "
                "Animation Clip in the "
                "Assets tab.");
        }

        return;
    }

    const std::string documentPath =
        ToUtf8(
            m_documentPath);

    const std::string texturePath =
        ToUtf8(
            m_data.texturePath);

    ImGui::TextUnformatted(
        "AnimationClip Document");

    ImGui::Separator();

    ImGui::TextDisabled(
        "Path");

    ImGui::TextWrapped(
        "%s",
        documentPath.empty()
        ? "<invalid path>"
        : documentPath.c_str());

    ImGui::Spacing();

    if (ImGui::Button(
        "Close"))
    {
        Close();

        return;
    }

    ImGui::SameLine();

    ImGui::TextDisabled(
        "Read-only in Stage 7. "
        "Editing / Save / Revert / "
        "Live Apply arrive in Stage 8.");

    ImGui::SeparatorText(
        "Clip");

    ImGui::Text(
        "Looping: %s",
        m_data.looping
        ? "true"
        : "false");

    ImGui::Text(
        "Frames: %zu",
        m_data.frames.size());

    ImGui::Text(
        "Total Duration: %.3f s",
        CalculateTotalDuration(
            m_data));

    ImGui::SeparatorText(
        "Texture");

    ImGui::TextWrapped(
        "%s",
        texturePath.empty()
        ? "<invalid texture path>"
        : texturePath.c_str());

    ImGui::SeparatorText(
        "Frames");

    constexpr ImGuiTableFlags
        tableFlags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable(
        "##AnimationFrames",
        6,
        tableFlags))
    {
        return;
    }

    ImGui::TableSetupColumn(
        "#",
        ImGuiTableColumnFlags_WidthFixed,
        40.0f);

    ImGui::TableSetupColumn(
        "U0");

    ImGui::TableSetupColumn(
        "V0");

    ImGui::TableSetupColumn(
        "U1");

    ImGui::TableSetupColumn(
        "V1");

    ImGui::TableSetupColumn(
        "Duration",
        ImGuiTableColumnFlags_WidthFixed,
        100.0f);

    ImGui::TableHeadersRow();

    for (std::size_t i = 0;
        i < m_data.frames.size();
        ++i)
    {
        const AnimationClipFrameData&
            frame =
            m_data.frames[i];

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::Text(
            "%zu",
            i);

        ImGui::TableSetColumnIndex(1);
        ImGui::Text(
            "%.6f",
            frame.uv.u0);

        ImGui::TableSetColumnIndex(2);
        ImGui::Text(
            "%.6f",
            frame.uv.v0);

        ImGui::TableSetColumnIndex(3);
        ImGui::Text(
            "%.6f",
            frame.uv.u1);

        ImGui::TableSetColumnIndex(4);
        ImGui::Text(
            "%.6f",
            frame.uv.v1);

        ImGui::TableSetColumnIndex(5);
        ImGui::Text(
            "%.3f",
            frame.duration);
    }

    ImGui::EndTable();
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