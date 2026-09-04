#pragma once

#include <memory>
#include <string>

class Tileset;
class ResourceManager;

class TilesetLoader
{
public:
    static std::unique_ptr<Tileset>
        Load(
            const std::wstring& path,
            ResourceManager& resources
        );
};