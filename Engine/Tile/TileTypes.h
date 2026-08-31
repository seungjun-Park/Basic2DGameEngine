#pragma once

#include <cstdint>

using TileId = std::uint32_t;

constexpr TileId InvalidTileId = 0;

struct UVRect
{
    float u0 = 0.0f;
    float v0 = 0.0f;

    float u1 = 1.0f;
    float v1 = 1.0f;
};