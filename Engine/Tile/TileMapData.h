#pragma once

#include "Engine/Renderer/RenderTypes.h"
#include "Engine/Tile/TileTypes.h"

#include <string>
#include <vector>

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

struct TileMapData
{
    int width = 0;
    int height = 0;

    int tileWidth = 0;
    int tileHeight = 0;

    std::wstring tilesetPath;

    std::vector<TileLayer> layers;
};