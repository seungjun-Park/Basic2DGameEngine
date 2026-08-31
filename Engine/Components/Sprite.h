#pragma once

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
};