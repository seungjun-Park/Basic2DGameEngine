#pragma once

#include <string>

struct TilesetData
{
    std::wstring texturePath;

    int tileWidth = 0;
    int tileHeight = 0;

    int columns = 0;
    int rows = 0;

    int margin = 0;
    int spacing = 0;
};