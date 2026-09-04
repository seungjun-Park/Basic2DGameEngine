#pragma once

#include "Engine/Graphics/UVRect.h"
#include "Engine/Renderer/RenderTypes.h"

class Texture;

struct Sprite
{
    Texture* texture = nullptr;

    bool visible = true;

    RenderLayer layer =
        RenderLayer::World;

    float zIndex = 0.0f;

    bool useYSort = false;

    BlendMode blendMode =
        BlendMode::Alpha;

    UVRect uv
    {
        0.0f,
        0.0f,
        1.0f,
        1.0f
    };
};