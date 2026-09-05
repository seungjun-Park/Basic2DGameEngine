#pragma once

#include <memory>
#include <string>

struct TileMapData;

class TileMapSerializer
{
public:
    static bool Save(
        const TileMapData& data,
        const std::wstring& path
    );

    static std::unique_ptr<TileMapData>
        Load(
            const std::wstring& path
        );

    [[nodiscard]]
    static bool Validate(
        const TileMapData& data
    ) noexcept;
};