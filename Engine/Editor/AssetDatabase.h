#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

enum class AssetType : std::uint8_t
{
    Unknown,

    Texture,
    AnimationClip,
    AudioClip,
    Tileset,
    TileMap,
    Scene,
    Shader
};

struct AssetRecord
{
    // Authoritative project-relative asset identity.
    std::wstring path;

    // Path relative to ProjectConfig::assetRoot.
    std::wstring relativePath;

    std::wstring fileName;
    std::wstring extension;

    AssetType type =
        AssetType::Unknown;

    std::uintmax_t fileSize = 0;
};

class AssetDatabase
{
public:
    AssetDatabase();
    ~AssetDatabase();

    AssetDatabase(
        const AssetDatabase&) = delete;

    AssetDatabase& operator=(
        const AssetDatabase&) = delete;

    AssetDatabase(
        AssetDatabase&&) = delete;

    AssetDatabase& operator=(
        AssetDatabase&&) = delete;

    bool Initialize(
        const std::wstring& assetRoot);

    void Shutdown();

    bool Refresh();

    [[nodiscard]]
    const AssetRecord* FindAsset(
        const std::wstring& path) const;

    [[nodiscard]]
    const std::vector<AssetRecord>&
        GetAssets() const noexcept;

    [[nodiscard]]
    const std::wstring&
        GetAssetRoot() const noexcept;

    [[nodiscard]]
    std::size_t
        GetAssetCount() const noexcept;

    [[nodiscard]]
    bool IsInitialized() const noexcept;

private:
    static AssetType DetermineAssetType(
        const std::filesystem::path&
        relativePath);

    static std::wstring NormalizePath(
        const std::filesystem::path& path);

    static std::wstring ToLower(
        std::wstring value);

    static bool IsValidAssetRoot(
        const std::filesystem::path& path);

private:
    std::filesystem::path
        m_assetRootPath;

    std::wstring m_assetRoot;

    std::vector<AssetRecord>
        m_assets;

    bool m_initialized = false;
};