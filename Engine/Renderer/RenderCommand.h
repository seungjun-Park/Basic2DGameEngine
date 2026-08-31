#pragma once

#include "RenderTypes.h"
#include "Engine/Tile/TileTypes.h"

#include <DirectXMath.h>
#include <cstdint>

class Texture;

struct SpriteRenderCommand
{
    Texture* texture = nullptr;

    DirectX::XMFLOAT2 position
    {
        0.0f,
        0.0f
    };

    DirectX::XMFLOAT2 scale
    {
        1.0f,
        1.0f
    };

    float rotation =
        0.0f;

    UVRect uv
    {
        0.0f,
        0.0f,
        1.0f,
        1.0f
    };

    RenderLayer layer =
        RenderLayer::World;

    BlendMode blendMode =
        BlendMode::Alpha;

    float sortZ =
        0.0f;

    std::uint64_t submissionOrder =
        0;
};