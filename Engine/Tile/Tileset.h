#pragma once

#include "TileTypes.h"

class Texture;

class Tileset
{
public:
    Tileset() = default;

    void SetTexture(
        Texture* texture
    );

    void SetTileSize(
        int width,
        int height
    );

    void SetGridSize(
        int columns,
        int rows
    );

    void SetMargin(
        int margin
    );

    void SetSpacing(
        int spacing
    );

    void ReplaceContents(
        const Tileset& source
    ) noexcept
    {
        if (this ==
            &source)
        {
            return;
        }

        m_texture =
            source.m_texture;

        m_tileWidth =
            source.m_tileWidth;

        m_tileHeight =
            source.m_tileHeight;

        m_columns =
            source.m_columns;

        m_rows =
            source.m_rows;

        m_margin =
            source.m_margin;

        m_spacing =
            source.m_spacing;
    }

    Texture* GetTexture() const;

    int GetTileWidth() const;
    int GetTileHeight() const;

    int GetColumns() const;
    int GetRows() const;

    int GetMargin() const;
    int GetSpacing() const;

    UVRect GetTileUV(
        TileId tileId
    ) const;

    bool IsValidTileId(
        TileId tileId
    ) const;

private:
    Texture* m_texture =
        nullptr;

    int m_tileWidth = 0;
    int m_tileHeight = 0;

    int m_columns = 0;
    int m_rows = 0;

    int m_margin = 0;
    int m_spacing = 0;
};