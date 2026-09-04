#pragma once

#include "RenderTypes.h"

#include <cstdint>

struct Sprite;
struct Transform;
class Texture;

struct SpriteRenderCommand
{
    const Sprite* sprite =
        nullptr;

    const Transform* transform =
        nullptr;

    Texture* texture =
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

inline bool IsSpriteRenderCommandValid(
    const SpriteRenderCommand& command) noexcept
{
    return
        command.sprite != nullptr &&
        command.transform != nullptr &&
        command.texture != nullptr;
}

inline bool CanBatchSpriteRenderCommands(
    const SpriteRenderCommand& a,
    const SpriteRenderCommand& b) noexcept
{
    if (!IsSpriteRenderCommandValid(a) ||
        !IsSpriteRenderCommandValid(b))
    {
        return false;
    }

    return
        a.layer == b.layer &&
        a.blendMode == b.blendMode &&
        a.texture == b.texture;
}