#pragma once

#include "TileTypes.h"

#include "Engine/Renderer/RenderTypes.h"

#include <string>
#include <vector>
#include <cstddef>

class Tileset;

enum class TileLayerType
{
    Render,
    Collision
};

struct TileLayer
{
    std::string name;

    TileLayerType type =
        TileLayerType::Render;

    RenderLayer renderLayer =
        RenderLayer::World;

    bool visible = true;

    std::vector<TileId> tiles;
};

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

    Tileset* m_tileset = nullptr;

    std::vector<TileLayer>
        m_layers;
};