#include "AssetBrowserPanel.h"

#include <Windows.h>

#include <limits>

AssetBrowserPanel::
AssetBrowserPanel() = default;

AssetBrowserPanel::
~AssetBrowserPanel() = default;

bool AssetBrowserPanel::DrawContents(
    AssetDatabase& assetDatabase,
    std::wstring& selectedAssetPath)
{
    bool openRequested = false;

    //
    // Toolbar
    //

    if (ImGui::Button(
        "Refresh"))
    {
        if (!assetDatabase.Refresh())
        {
            m_lastRefreshFailed = true;
        }
        else
        {
            m_lastRefreshFailed = false;

            if (!selectedAssetPath.empty() &&
                !assetDatabase.FindAsset(
                    selectedAssetPath))
            {
                selectedAssetPath.clear();
            }
        }
    }

    ImGui::SameLine();

    ImGui::TextDisabled(
        "%zu assets",
        assetDatabase.GetAssetCount());

    if (m_lastRefreshFailed)
    {
        ImGui::SameLine();

        ImGui::TextDisabled(
            "Refresh failed.");
    }

    ImGui::SameLine();

    ImGui::TextDisabled(
        "Double-click an Animation Clip "
        "to open it.");

    //
    // Search
    //

    m_searchFilter.Draw(
        "Search",
        300.0f);

    ImGui::Separator();

    //
    // Asset table
    //

    constexpr ImGuiTableFlags tableFlags =
        ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable(
        "##AssetBrowserTable",
        4,
        tableFlags))
    {
        ImGui::TableSetupColumn(
            "Name",
            ImGuiTableColumnFlags_WidthStretch,
            0.30f);

        ImGui::TableSetupColumn(
            "Type",
            ImGuiTableColumnFlags_WidthFixed,
            120.0f);

        ImGui::TableSetupColumn(
            "Path",
            ImGuiTableColumnFlags_WidthStretch,
            0.55f);

        ImGui::TableSetupColumn(
            "Size",
            ImGuiTableColumnFlags_WidthFixed,
            90.0f);

        ImGui::TableHeadersRow();

        const auto& assets =
            assetDatabase.GetAssets();

        for (std::size_t index = 0;
            index < assets.size();
            ++index)
        {
            const AssetRecord& asset =
                assets[index];

            const std::string fileName =
                ToUtf8(
                    asset.fileName);

            const std::string path =
                ToUtf8(
                    asset.path);

            if (!m_searchFilter.PassFilter(
                fileName.c_str()) &&
                !m_searchFilter.PassFilter(
                    path.c_str()))
            {
                continue;
            }

            ImGui::PushID(
                static_cast<int>(
                    index));

            ImGui::TableNextRow();

            //
            // Name / Selection / Open
            //

            ImGui::TableSetColumnIndex(
                0);

            const bool selected =
                selectedAssetPath ==
                asset.path;

            const char* displayName =
                fileName.empty()
                ? "<invalid filename>"
                : fileName.c_str();

            if (ImGui::Selectable(
                displayName,
                selected,
                ImGuiSelectableFlags_AllowDoubleClick))
            {
                selectedAssetPath =
                    asset.path;
            }

            if (asset.type ==
                AssetType::AnimationClip &&
                ImGui::IsItemHovered() &&
                ImGui::IsMouseDoubleClicked(
                    ImGuiMouseButton_Left))
            {
                selectedAssetPath =
                    asset.path;

                openRequested =
                    true;
            }

            //
            // Type
            //

            ImGui::TableSetColumnIndex(
                1);

            ImGui::TextUnformatted(
                GetAssetTypeLabel(
                    asset.type));

            //
            // Path
            //

            ImGui::TableSetColumnIndex(
                2);

            ImGui::TextUnformatted(
                path.empty()
                ? "<invalid path>"
                : path.c_str());

            //
            // Size
            //

            ImGui::TableSetColumnIndex(
                3);

            DrawFileSize(
                asset.fileSize);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    //
    // Selection details
    //

    ImGui::Separator();

    ImGui::TextUnformatted(
        "Selected Asset");

    if (selectedAssetPath.empty())
    {
        ImGui::TextDisabled(
            "None");

        return openRequested;
    }

    const AssetRecord* selectedAsset =
        assetDatabase.FindAsset(
            selectedAssetPath);

    if (!selectedAsset)
    {
        selectedAssetPath.clear();

        ImGui::TextDisabled(
            "Selected asset no longer exists.");

        return openRequested;
    }

    const std::string selectedPath =
        ToUtf8(
            selectedAsset->path);

    ImGui::TextWrapped(
        "%s",
        selectedPath.empty()
        ? "<invalid path>"
        : selectedPath.c_str());

    ImGui::Text(
        "Type: %s",
        GetAssetTypeLabel(
            selectedAsset->type));

    if (selectedAsset->type ==
        AssetType::AnimationClip)
    {
        if (ImGui::Button(
            "Open Animation Clip"))
        {
            openRequested =
                true;
        }
    }
    else
    {
        ImGui::TextDisabled(
            "This asset type does not "
            "have an editor yet.");
    }

    return openRequested;
}

void AssetBrowserPanel::Reset()
{
    m_searchFilter.Clear();

    m_lastRefreshFailed = false;
}

const char*
AssetBrowserPanel::GetAssetTypeLabel(
    AssetType type) noexcept
{
    switch (type)
    {
    case AssetType::Texture:
        return "Texture";

    case AssetType::AnimationClip:
        return "Animation Clip";

    case AssetType::AudioClip:
        return "Audio Clip";

    case AssetType::Tileset:
        return "Tileset";

    case AssetType::TileMap:
        return "TileMap";

    case AssetType::Scene:
        return "Scene";

    case AssetType::Shader:
        return "Shader";

    case AssetType::Unknown:
    default:
        return "Unknown";
    }
}

std::string
AssetBrowserPanel::ToUtf8(
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
            value.size()
            );

    const int requiredSize =
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            nullptr,
            0,
            nullptr,
            nullptr
        );

    if (requiredSize <= 0)
    {
        return {};
    }

    std::string result(
        static_cast<std::size_t>(
            requiredSize),
        '\0'
    );

    const int convertedSize =
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            result.data(),
            requiredSize,
            nullptr,
            nullptr
        );

    if (convertedSize !=
        requiredSize)
    {
        return {};
    }

    return result;
}

void AssetBrowserPanel::DrawFileSize(
    std::uintmax_t fileSize)
{
    constexpr double bytesPerKilobyte =
        1024.0;

    constexpr double bytesPerMegabyte =
        bytesPerKilobyte *
        1024.0;

    const double size =
        static_cast<double>(
            fileSize
            );

    if (size >=
        bytesPerMegabyte)
    {
        ImGui::Text(
            "%.2f MB",
            size /
            bytesPerMegabyte
        );

        return;
    }

    if (size >=
        bytesPerKilobyte)
    {
        ImGui::Text(
            "%.1f KB",
            size /
            bytesPerKilobyte
        );

        return;
    }

    ImGui::Text(
        "%llu B",
        static_cast<
        unsigned long long>(
            fileSize
            )
    );
}