#pragma once

#include <memory>
#include <string>

struct TilesetData;

class TilesetSerializer
{
public:
    static bool Save(
        const TilesetData& data,
        const std::wstring& path
    );

    static std::unique_ptr<TilesetData>
        Load(
            const std::wstring& path
        );

    [[nodiscard]]
    static bool Validate(
        const TilesetData& data
    ) noexcept;
};