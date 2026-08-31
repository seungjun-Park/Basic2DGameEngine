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
    // Invalid UV는 모든 값 0.
    //
    // 기존 UVRect 기본값은
    // u1/v1이 1이기 때문에
    // 단순히 UVRect{}를 반환하면
    // 전체 texture가 선택될 수 있다.
    UVRect result
    {
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    if (!IsValidTileId(
        tileId))
    {
        return result;
    }

    const TileId zeroBased =
        tileId - 1;

    const int tileX =
        static_cast<int>(
            zeroBased %
            static_cast<TileId>(
                m_columns
                )
            );

    const int tileY =
        static_cast<int>(
            zeroBased /
            static_cast<TileId>(
                m_columns
                )
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
        static_cast<float>(
            tileX
            ) *
        invColumns;

    result.v0 =
        static_cast<float>(
            tileY
            ) *
        invRows;

    result.u1 =
        result.u0 +
        invColumns;

    result.v1 =
        result.v0 +
        invRows;

    return result;
}

bool Tileset::IsValidTileId(
    TileId tileId) const
{
    if (tileId == InvalidTileId)
    {
        return false;
    }

    if (m_columns <= 0 ||
        m_rows <= 0)
    {
        return false;
    }

    const TileId tileCount =
        static_cast<TileId>(
            m_columns * m_rows
            );

    return
        tileId <= tileCount;
}