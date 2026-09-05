#pragma once

#include "Engine/Components/Transform.h"
#include "Engine/Graphics/UVRect.h"
#include "Engine/Renderer/RenderTypes.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <utility>

struct SerializedSprite
{
    std::wstring texturePath;

    bool visible = true;

    RenderLayer layer =
        RenderLayer::World;

    float zIndex = 0.0f;

    bool useYSort = false;

    BlendMode blendMode =
        BlendMode::Alpha;

    UVRect uv{};
};

struct SerializedEntity
{
    std::string type;

    bool active = true;

    Transform transform;

    std::optional<SerializedSprite>
        sprite;
};

struct SerializedAnimationBinding
{
    std::string slot;

    std::wstring clipPath;
};

struct SceneData
{
    static constexpr std::uint32_t
        LegacyVersion = 1;

    static constexpr std::uint32_t
        CurrentVersion = 2;

    std::uint32_t version =
        CurrentVersion;

    std::wstring tileMapPath;

    std::vector<SerializedAnimationBinding>
        animationBindings;

    std::vector<SerializedEntity>
        entities;
};