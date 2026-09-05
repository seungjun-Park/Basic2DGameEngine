#pragma once

#include "Engine/Editor/AssetDatabase.h"

#include <imgui.h>

#include <string>

class AssetBrowserPanel
{
public:
    AssetBrowserPanel();
    ~AssetBrowserPanel();

    AssetBrowserPanel(
        const AssetBrowserPanel&) = delete;

    AssetBrowserPanel& operator=(
        const AssetBrowserPanel&) = delete;

    AssetBrowserPanel(
        AssetBrowserPanel&&) = delete;

    AssetBrowserPanel& operator=(
        AssetBrowserPanel&&) = delete;

    bool DrawContents(
        AssetDatabase& assetDatabase,
        std::wstring& selectedAssetPath);

    void Reset();

private:
    static const char*
        GetAssetTypeLabel(
            AssetType type) noexcept;

    static std::string
        ToUtf8(
            const std::wstring& value);

    static void
        DrawFileSize(
            std::uintmax_t fileSize);

private:
    ImGuiTextFilter m_searchFilter;

    bool m_lastRefreshFailed = false;
};