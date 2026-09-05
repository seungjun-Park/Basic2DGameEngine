#pragma once

#include <string>

struct TileMapData;

class ITileMapRuntimeTarget
{
public:
    virtual ~ITileMapRuntimeTarget() = default;

    [[nodiscard]]
    virtual bool IsUsingTileMap(
        const std::wstring& path
    ) const noexcept = 0;

    virtual bool ApplyTileMapData(
        const std::wstring& path,
        const TileMapData& data
    ) = 0;
};