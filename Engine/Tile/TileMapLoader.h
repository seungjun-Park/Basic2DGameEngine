#pragma once
#pragma once

#include <memory>
#include <string>
#include "TileMap.h"

class ResourceManager;

class TileMapLoader
{
public:
    static std::unique_ptr<TileMap>
        Load(
            const std::wstring& path,
            ResourceManager& resources
        );
};