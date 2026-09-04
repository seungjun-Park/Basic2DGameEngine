#include "Tileset.h"

#include "Engine/Graphics/Texture.h"

#include <cstdint>

void Tileset::SetTexture(
    Texture* texture)
{
    m_texture =
        texture;
}

void Tileset::SetTileSize(
    int width,
    int height)
{
    m_tileWidth =
        width;

    m_tileHeight =
        height;
}

void Tileset::SetGridSize(
    int columns,
    int rows)
{
    m_columns =
        columns;

    m_rows =
        rows;
}

void Tileset::SetMargin(
    int margin)
{
    m_margin =
        margin;
}

void Tileset::SetSpacing(
    int spacing)
{
    m_spacing =
        spacing;
}

Texture* Tileset::GetTexture() const
{
    return
        m_texture;
}

int Tileset::GetTileWidth() const
{
    return
        m_tileWidth;
}

int Tileset::GetTileHeight() const
{
    return
        m_tileHeight;
}

int Tileset::GetColumns() const
{
    return
        m_columns;
}

int Tileset::GetRows() const
{
    return
        m_rows;
}

int Tileset::GetMargin() const
{
    return
        m_margin;
}

int Tileset::GetSpacing() const
{
    return
        m_spacing;
}

UVRect Tileset::GetTileUV(
    TileId tileId) const
{
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

    if (!m_texture)
    {
        return result;
    }

    if (m_tileWidth <= 0 ||
        m_tileHeight <= 0)
    {
        return result;
    }

    if (m_margin < 0 ||
        m_spacing < 0)
    {
        return result;
    }

    const int textureWidth =
        m_texture->GetWidth();

    const int textureHeight =
        m_texture->GetHeight();

    if (textureWidth <= 0 ||
        textureHeight <= 0)
    {
        return result;
    }

    const std::uint64_t
        zeroBased =
        static_cast<std::uint64_t>(
            tileId - 1
            );

    const std::uint64_t
        column =
        zeroBased
        %
        static_cast<std::uint64_t>(
            m_columns
            );

    const std::uint64_t
        row =
        zeroBased
        /
        static_cast<std::uint64_t>(
            m_columns
            );

    const std::int64_t
        strideX =
        static_cast<std::int64_t>(
            m_tileWidth
            )
        +
        static_cast<std::int64_t>(
            m_spacing
            );

    const std::int64_t
        strideY =
        static_cast<std::int64_t>(
            m_tileHeight
            )
        +
        static_cast<std::int64_t>(
            m_spacing
            );

    const std::int64_t
        pixelX =
        static_cast<std::int64_t>(
            m_margin
            )
        +
        static_cast<std::int64_t>(
            column
            )
        *
        strideX;

    const std::int64_t
        pixelY =
        static_cast<std::int64_t>(
            m_margin
            )
        +
        static_cast<std::int64_t>(
            row
            )
        *
        strideY;

    const std::int64_t
        pixelRight =
        pixelX
        +
        static_cast<std::int64_t>(
            m_tileWidth
            );

    const std::int64_t
        pixelBottom =
        pixelY
        +
        static_cast<std::int64_t>(
            m_tileHeight
            );

    if (pixelX < 0 ||
        pixelY < 0)
    {
        return result;
    }

    if (pixelRight >
        static_cast<std::int64_t>(
            textureWidth
            ) ||
        pixelBottom >
        static_cast<std::int64_t>(
            textureHeight
            ))
    {
        return result;
    }

    const float
        inverseTextureWidth =
        1.0f /
        static_cast<float>(
            textureWidth
            );

    const float
        inverseTextureHeight =
        1.0f /
        static_cast<float>(
            textureHeight
            );

    result.u0 =
        static_cast<float>(
            pixelX
            )
        *
        inverseTextureWidth;

    result.v0 =
        static_cast<float>(
            pixelY
            )
        *
        inverseTextureHeight;

    result.u1 =
        static_cast<float>(
            pixelRight
            )
        *
        inverseTextureWidth;

    result.v1 =
        static_cast<float>(
            pixelBottom
            )
        *
        inverseTextureHeight;

    return result;
}

bool Tileset::IsValidTileId(
    TileId tileId) const
{
    if (tileId ==
        InvalidTileId)
    {
        return false;
    }

    if (m_columns <= 0 ||
        m_rows <= 0)
    {
        return false;
    }

    const std::uint64_t
        tileCount =
        static_cast<std::uint64_t>(
            m_columns
            )
        *
        static_cast<std::uint64_t>(
            m_rows
            );

    return
        static_cast<std::uint64_t>(
            tileId
            )
        <=
        tileCount;
}