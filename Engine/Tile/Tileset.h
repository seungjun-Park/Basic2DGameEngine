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

    Texture* GetTexture() const;

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

    int GetTileWidth() const;
    int GetTileHeight() const;

    int GetColumns() const;
    int GetRows() const;

    int GetMargin() const;
    int GetSpacing() const;

    bool IsValidTileId(
        TileId tileId
    ) const;

    UVRect GetTileUV(
        TileId tileId
    ) const;

private:
    Texture* m_texture = nullptr;

    int m_tileWidth = 0;
    int m_tileHeight = 0;

    int m_columns = 0;
    int m_rows = 0;

    int m_margin = 0;
    int m_spacing = 0;
};