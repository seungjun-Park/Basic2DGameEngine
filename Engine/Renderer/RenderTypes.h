#pragma once

#include <cstdint>

enum class RenderLayer : std::int32_t
{
    Background = 0,
    World = 100,
    Effect = 200,
    Foreground = 300,
    UI = 1000,
    Debug = 2000
};

enum class BlendMode
{
    Opaque,
    Alpha
};