#include "AssetDatabase.h"

#include "Engine/Debug/DebugLog.h"

#include <algorithm>
#include <cwctype>
#include <system_error>
#include <utility>

AssetDatabase::AssetDatabase() = default;

AssetDatabase::~AssetDatabase()
{
    Shutdown();
}

bool AssetDatabase::Initialize(
    const std::wstring& assetRoot)
{
    if (m_initialized)
    {
        return false;
    }

    if (assetRoot.empty())
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Asset root is empty.\n"
        );

        return false;
    }

    const std::filesystem::path
        rootPath =
        std::filesystem::path(
            assetRoot
        ).lexically_normal();

    if (!IsValidAssetRoot(
        rootPath))
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Asset root must be "
            "project-relative.\n"
        );

        return false;
    }

    std::error_code error;

    if (!std::filesystem::exists(
        rootPath,
        error))
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Asset root does not exist.\n"
        );

        return false;
    }

    if (error)
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Failed to inspect asset root.\n"
        );

        return false;
    }

    if (!std::filesystem::is_directory(
        rootPath,
        error))
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Asset root is not a directory.\n"
        );

        return false;
    }

    if (error)
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Failed to inspect asset root.\n"
        );

        return false;
    }

    m_assetRootPath =
        rootPath;

    m_assetRoot =
        NormalizePath(
            rootPath
        );

    m_initialized = true;

    if (!Refresh())
    {
        Shutdown();

        return false;
    }

    ENGINE_DEBUG_LOG(
        "[AssetDatabase] "
        "Initialized.\n"
    );

    return true;
}

void AssetDatabase::Shutdown()
{
    m_assets.clear();

    m_assetRoot.clear();

    m_assetRootPath.clear();

    m_initialized = false;
}

bool AssetDatabase::Refresh()
{
    if (!m_initialized)
    {
        return false;
    }

    std::vector<AssetRecord>
        discoveredAssets;

    std::error_code error;

    const auto options =
        std::filesystem::
        directory_options::
        skip_permission_denied;

    std::filesystem::
        recursive_directory_iterator
        iterator(
            m_assetRootPath,
            options,
            error
        );

    const std::filesystem::
        recursive_directory_iterator
        end;

    if (error)
    {
        ENGINE_DEBUG_LOG(
            "[AssetDatabase] "
            "Failed to start asset scan.\n"
        );

        return false;
    }

    while (iterator != end)
    {
        const std::filesystem::
            directory_entry&
            entry =
            *iterator;

        const auto status =
            entry.symlink_status(
                error
            );

        if (error)
        {
            ENGINE_DEBUG_LOG(
                "[AssetDatabase] "
                "Failed to inspect asset entry.\n"
            );

            return false;
        }

        // AssetDatabase does not follow or
        // register symbolic links.
        if (!std::filesystem::
            is_symlink(status))
        {
            const bool isRegularFile =
                entry.is_regular_file(
                    error
                );

            if (error)
            {
                ENGINE_DEBUG_LOG(
                    "[AssetDatabase] "
                    "Failed to inspect asset file.\n"
                );

                return false;
            }

            if (isRegularFile)
            {
                const std::filesystem::path
                    filePath =
                    entry.path().
                    lexically_normal();

                const std::filesystem::path
                    relativePath =
                    std::filesystem::relative(
                        filePath,
                        m_assetRootPath,
                        error
                    );

                if (error)
                {
                    ENGINE_DEBUG_LOG(
                        "[AssetDatabase] "
                        "Failed to create "
                        "relative asset path.\n"
                    );

                    return false;
                }

                const std::uintmax_t
                    fileSize =
                    entry.file_size(
                        error
                    );

                if (error)
                {
                    ENGINE_DEBUG_LOG(
                        "[AssetDatabase] "
                        "Failed to read asset "
                        "file size.\n"
                    );

                    return false;
                }

                AssetRecord record;

                record.path =
                    NormalizePath(
                        filePath
                    );

                record.relativePath =
                    NormalizePath(
                        relativePath
                    );

                record.fileName =
                    filePath.
                    filename().
                    wstring();

                record.extension =
                    ToLower(
                        filePath.
                        extension().
                        wstring()
                    );

                record.type =
                    DetermineAssetType(
                        relativePath
                    );

                record.fileSize =
                    fileSize;

                discoveredAssets.
                    push_back(
                        std::move(
                            record
                        )
                    );
            }
        }

        iterator.increment(
            error
        );

        if (error)
        {
            ENGINE_DEBUG_LOG(
                "[AssetDatabase] "
                "Asset scan failed.\n"
            );

            return false;
        }
    }

    std::sort(
        discoveredAssets.begin(),
        discoveredAssets.end(),
        [](
            const AssetRecord& lhs,
            const AssetRecord& rhs)
        {
            return
                lhs.path <
                rhs.path;
        }
    );

    m_assets =
        std::move(
            discoveredAssets
        );

    ENGINE_DEBUG_LOG(
        "[AssetDatabase] "
        "Asset scan completed.\n"
    );

    return true;
}

const AssetRecord*
AssetDatabase::FindAsset(
    const std::wstring& path) const
{
    if (!m_initialized ||
        path.empty())
    {
        return nullptr;
    }

    const std::wstring
        normalizedPath =
        NormalizePath(
            std::filesystem::path(
                path
            ).
            lexically_normal()
        );

    const auto found =
        std::find_if(
            m_assets.begin(),
            m_assets.end(),
            [&normalizedPath](
                const AssetRecord& asset)
            {
                return
                    asset.path ==
                    normalizedPath;
            }
        );

    if (found ==
        m_assets.end())
    {
        return nullptr;
    }

    return &(*found);
}

const std::vector<AssetRecord>&
AssetDatabase::GetAssets()
const noexcept
{
    return m_assets;
}

const std::wstring&
AssetDatabase::GetAssetRoot()
const noexcept
{
    return m_assetRoot;
}

std::size_t
AssetDatabase::GetAssetCount()
const noexcept
{
    return m_assets.size();
}

bool AssetDatabase::IsInitialized()
const noexcept
{
    return m_initialized;
}

AssetType
AssetDatabase::DetermineAssetType(
    const std::filesystem::path&
    relativePath)
{
    const std::wstring extension =
        ToLower(
            relativePath.
            extension().
            wstring()
        );

    //
    // File formats with an unambiguous
    // extension.
    //

    if (extension == L".png" ||
        extension == L".jpg" ||
        extension == L".jpeg" ||
        extension == L".bmp" ||
        extension == L".gif" ||
        extension == L".tif" ||
        extension == L".tiff")
    {
        return AssetType::Texture;
    }

    if (extension == L".wav")
    {
        return AssetType::AudioClip;
    }

    if (extension == L".hlsl")
    {
        return AssetType::Shader;
    }

    //
    // Current serialized resource formats
    // share .json, therefore classify them
    // from the first directory below
    // assetRoot.
    //

    if (extension != L".json")
    {
        return AssetType::Unknown;
    }

    const auto first =
        relativePath.begin();

    if (first ==
        relativePath.end())
    {
        return AssetType::Unknown;
    }

    const std::wstring
        category =
        ToLower(
            first->wstring()
        );

    if (category ==
        L"animations")
    {
        return
            AssetType::AnimationClip;
    }

    if (category ==
        L"tilesets")
    {
        return
            AssetType::Tileset;
    }

    if (category == L"maps" ||
        category == L"tilemaps")
    {
        return
            AssetType::TileMap;
    }

    if (category ==
        L"scenes")
    {
        return
            AssetType::Scene;
    }

    return AssetType::Unknown;
}

std::wstring
AssetDatabase::NormalizePath(
    const std::filesystem::path& path)
{
    return
        path.
        lexically_normal().
        generic_wstring();
}

std::wstring
AssetDatabase::ToLower(
    std::wstring value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](
            wchar_t character)
        {
            return
                static_cast<wchar_t>(
                    std::towlower(
                        character
                    )
                    );
        }
    );

    return value;
}

bool AssetDatabase::IsValidAssetRoot(
    const std::filesystem::path& path)
{
    if (path.empty() ||
        path.is_absolute())
    {
        return false;
    }

    const auto first =
        path.begin();

    if (first != path.end() &&
        *first ==
        std::filesystem::path(
            L".."
        ))
    {
        return false;
    }

    return true;
}