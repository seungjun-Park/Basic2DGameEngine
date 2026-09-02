#pragma once

#include "Engine/Graphics/UVRect.h"

#include <cstdint>

using TileId = std::uint32_t;

constexpr TileId InvalidTileId = 0;

// Collision Layer contract:
//
// 0 = Empty
// 1 = Solid
// 2+ = Invalid
//
// Collision TileId는 Tileset index가 아니다.
constexpr TileId EmptyCollisionTile = 0;
constexpr TileId SolidCollisionTile = 1;

constexpr bool IsSolidCollisionTile(
    TileId tileId) noexcept
{
    return tileId == SolidCollisionTile;
}