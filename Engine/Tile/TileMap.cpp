#include "TileMap.h"

#include "Tileset.h"

#include <utility>

void TileMap::SetSize(
    int width,
    int height)
{
    m_width = width;
    m_height = height;
}

void TileMap::SetTileSize(
    int tileWidth,
    int tileHeight)
{
    m_tileWidth =
        tileWidth;

    m_tileHeight =
        tileHeight;
}

void TileMap::SetTileset(
    Tileset* tileset)
{
    m_tileset =
        tileset;
}

void TileMap::SetTile(
    std::size_t layerIndex,
    int x,
    int y,
    TileId tileId)
{
    if (layerIndex >=
        m_layers.size())
    {
        return;
    }

    if (!IsInside(x, y))
    {
        return;
    }

    TileLayer& layer =
        m_layers[layerIndex];

    const std::size_t expectedSize =
        static_cast<std::size_t>(
            m_width * m_height
            );

    if (layer.tiles.size() <
        expectedSize)
    {
        layer.tiles.resize(
            expectedSize,
            InvalidTileId
        );
    }

    const std::size_t index =
        static_cast<std::size_t>(
            y * m_width + x
            );

    layer.tiles[index] =
        tileId;
}

void TileMap::AddLayer(
    TileLayer layer)
{
    const std::size_t expectedSize =
        static_cast<std::size_t>(
            m_width * m_height
            );

    if (layer.tiles.size() <
        expectedSize)
    {
        layer.tiles.resize(
            expectedSize,
            InvalidTileId
        );
    }

    m_layers.emplace_back(
        std::move(layer)
    );
}

int TileMap::GetWidth() const
{
    return m_width;
}

int TileMap::GetHeight() const
{
    return m_height;
}

int TileMap::GetTileWidth() const
{
    return m_tileWidth;
}

int TileMap::GetTileHeight() const
{
    return m_tileHeight;
}

Tileset*
TileMap::GetTileset() const
{
    return m_tileset;
}

TileId TileMap::GetTile(
    std::size_t layerIndex,
    int x,
    int y) const
{
    if (layerIndex >=
        m_layers.size())
    {
        return InvalidTileId;
    }

    if (!IsInside(x, y))
    {
        return InvalidTileId;
    }

    const TileLayer& layer =
        m_layers[layerIndex];

    const std::size_t index =
        static_cast<std::size_t>(
            y * m_width + x
            );

    if (index >=
        layer.tiles.size())
    {
        return InvalidTileId;
    }

    return
        layer.tiles[index];
}

TileLayer*
TileMap::GetLayer(
    std::size_t index)
{
    if (index >=
        m_layers.size())
    {
        return nullptr;
    }

    return
        &m_layers[index];
}

const TileLayer*
TileMap::GetLayer(
    std::size_t index) const
{
    if (index >=
        m_layers.size())
    {
        return nullptr;
    }

    return
        &m_layers[index];
}

std::size_t
TileMap::GetLayerCount() const
{
    return
        m_layers.size();
}

bool TileMap::IsInside(
    int x,
    int y) const
{
    return
        x >= 0 &&
        y >= 0 &&
        x < m_width &&
        y < m_height;
}