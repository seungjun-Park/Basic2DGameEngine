#pragma once

#include "RenderTypes.h"

#include <cstdint>

struct Sprite;
struct Transform;

struct SpriteRenderCommand
{
    const Sprite* sprite =
        nullptr;

    const Transform* transform =
        nullptr;

    RenderLayer layer =
        RenderLayer::World;

    BlendMode blendMode =
        BlendMode::Alpha;

    float sortZ =
        0.0f;

    std::uint64_t submissionOrder =
        0;
};