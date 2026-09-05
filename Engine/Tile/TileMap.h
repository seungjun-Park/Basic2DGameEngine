#pragma once

#include "Engine/Tile/TileMapData.h"

#include <cstddef>
#include <utility>
#include <vector>

class Tileset;

class TileMap
{
public:
    TileMap() = default;

    void SetSize(
        int width,
        int height
    );

    void SetTileSize(
        int tileWidth,
        int tileHeight
    );

    void SetTileset(
        Tileset* tileset
    );

    void SetTile(
        std::size_t layerIndex,
        int x,
        int y,
        TileId tileId
    );

    void AddLayer(
        TileLayer layer
    );

    void ReplaceContents(
        TileMap&& source
    ) noexcept
    {
        if (this ==
            &source)
        {
            return;
        }

        m_width =
            source.m_width;

        m_height =
            source.m_height;

        m_tileWidth =
            source.m_tileWidth;

        m_tileHeight =
            source.m_tileHeight;

        m_tileset =
            source.m_tileset;

        m_layers =
            std::move(
                source.m_layers
            );

        source.m_width = 0;
        source.m_height = 0;

        source.m_tileWidth = 0;
        source.m_tileHeight = 0;

        source.m_tileset =
            nullptr;

        source.m_layers.clear();
    }

    int GetWidth() const;
    int GetHeight() const;

    int GetTileWidth() const;
    int GetTileHeight() const;

    Tileset* GetTileset() const;

    TileId GetTile(
        std::size_t layerIndex,
        int x,
        int y
    ) const;

    TileLayer* GetLayer(
        std::size_t index
    );

    const TileLayer* GetLayer(
        std::size_t index
    ) const;

    std::size_t GetLayerCount() const;

private:
    bool IsInside(
        int x,
        int y
    ) const;

private:
    int m_width = 0;
    int m_height = 0;

    int m_tileWidth = 0;
    int m_tileHeight = 0;

    Tileset* m_tileset =
        nullptr;

    std::vector<TileLayer>
        m_layers;
};