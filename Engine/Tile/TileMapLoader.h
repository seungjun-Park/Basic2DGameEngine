#pragma once

#include <memory>
#include <string>

class ResourceManager;
class TileMap;

class TileMapLoader
{
public:
    static std::unique_ptr<TileMap>
        Load(
            const std::wstring& path,
            ResourceManager& resources
        );
};