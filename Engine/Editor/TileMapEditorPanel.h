#pragma once

#include "Engine/Tile/TileMapData.h"
#include "Engine/Tile/TilesetData.h"

#include <cstddef>
#include <limits>
#include <string>

class ResourceManager;
class Texture;

class TileMapEditorPanel
{
public:
    bool Open(
        const std::wstring& path,
        ResourceManager& resourceManager
    );

    void Close() noexcept;

    void DrawContents(
        ResourceManager& resourceManager
    );

    [[nodiscard]]
    bool IsOpen() const noexcept;

    [[nodiscard]]
    const std::wstring&
        GetDocumentPath() const noexcept;

    [[nodiscard]]
    const std::wstring&
        GetTilesetPath() const noexcept;

    [[nodiscard]]
    std::size_t
        GetSelectedLayerIndex() const noexcept;

    [[nodiscard]]
    bool HasSelectedLayer() const noexcept;

private:
    bool Reload(
        ResourceManager& resourceManager
    );

    void DrawDocumentInfo();

    void DrawLayers();

    void DrawSelectedLayerInfo();

    [[nodiscard]]
    bool ValidateAgainstTileset(
        const TileMapData& mapData,
        const TilesetData& tilesetData,
        const Texture& texture
    ) const noexcept;

    static const char*
        GetLayerTypeName(
            TileLayerType type
        ) noexcept;

    static const char*
        GetRenderLayerName(
            RenderLayer layer
        ) noexcept;

    static std::string ToUtf8(
        const std::wstring& value
    );

private:
    static constexpr std::size_t
        InvalidLayerIndex =
        std::numeric_limits<
        std::size_t
        >::max();

private:
    std::wstring
        m_documentPath;

    TileMapData
        m_data{};

    TilesetData
        m_tilesetData{};

    Texture* m_texture =
        nullptr;

    std::size_t
        m_selectedLayerIndex =
        InvalidLayerIndex;

    bool m_lastOpenFailed =
        false;
};