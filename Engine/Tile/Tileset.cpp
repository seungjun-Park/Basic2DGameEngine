#include "Tileset.h"

#include "Engine/Graphics/Texture.h"

void Tileset::SetTexture(
    Texture* texture)
{
    m_texture = texture;
}

Texture* Tileset::GetTexture() const
{
    return m_texture;
}

void Tileset::SetTileSize(
    int width,
    int height)
{
    m_tileWidth = width;
    m_tileHeight = height;
}

void Tileset::SetGridSize(
    int columns,
    int rows)
{
    m_columns = columns;
    m_rows = rows;
}

int Tileset::GetTileWidth() const
{
    return m_tileWidth;
}

int Tileset::GetTileHeight() const
{
    return m_tileHeight;
}

int Tileset::GetColumns() const
{
    return m_columns;
}

int Tileset::GetRows() const
{
    return m_rows;
}

UVRect Tileset::GetTileUV(
    TileId tileId) const
{
    UVRect result{};

    if (tileId == InvalidTileId)
    {
        return result;
    }

    if (m_columns <= 0 ||
        m_rows <= 0)
    {
        return result;
    }

    // Tile ID는 1부터 시작한다고 가정
    const TileId zeroBased =
        tileId - 1;

    const int tileX =
        static_cast<int>(
            zeroBased %
            m_columns
            );

    const int tileY =
        static_cast<int>(
            zeroBased /
            m_columns
            );

    const float invColumns =
        1.0f /
        static_cast<float>(
            m_columns
            );

    const float invRows =
        1.0f /
        static_cast<float>(
            m_rows
            );

    result.u0 =
        tileX *
        invColumns;

    result.v0 =
        tileY *
        invRows;

    result.u1 =
        result.u0 +
        invColumns;

    result.v1 =
        result.v0 +
        invRows;

    return result;
}