#pragma once

#include "Engine/Tile/TileTypes.h"
#include "Engine/Tile/TilesetData.h"

#include <string>

class AssetDatabase;
class ResourceManager;
class Texture;

class TilePalettePanel
{
public:
    bool OpenTileset(
        const std::wstring& path,
        ResourceManager& resourceManager
    );

    void Close() noexcept;

    void DrawContents(
        const AssetDatabase& assetDatabase,
        const std::wstring& selectedAssetPath,
        ResourceManager& resourceManager
    );

    [[nodiscard]]
    bool IsOpen() const noexcept;

    [[nodiscard]]
    TileId GetSelectedTileId() const noexcept;

    [[nodiscard]]
    bool HasSelectedTile() const noexcept;

    [[nodiscard]]
    const std::wstring&
        GetTilesetPath() const noexcept;

private:
    void DrawPalette();

    [[nodiscard]]
    bool ValidateAgainstTexture(
        const TilesetData& data,
        const Texture& texture
    ) const noexcept;

    [[nodiscard]]
    UVRect CalculateTileUV(
        TileId tileId
    ) const noexcept;

    [[nodiscard]]
    std::uint64_t
        GetTileCount() const noexcept;

    static std::string ToUtf8(
        const std::wstring& value
    );

private:
    std::wstring m_tilesetPath;

    TilesetData m_tilesetData{};

    Texture* m_texture = nullptr;

    TileId m_selectedTileId =
        InvalidTileId;

    float m_thumbnailSize = 64.0f;

    bool m_lastOpenFailed = false;
};