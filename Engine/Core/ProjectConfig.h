#pragma once

#include "EngineConfig.h"

#include <cstdint>
#include <string>

struct ProjectConfig
{
    static constexpr std::uint32_t
        CurrentVersion = 1;

    std::uint32_t version =
        CurrentVersion;

    EngineConfig engine{};

    std::wstring assetRoot;

    std::wstring startScene;
};