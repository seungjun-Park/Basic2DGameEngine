#pragma once

#include "Engine/Components/Transform.h"
#include "Engine/Graphics/UVRect.h"
#include "Engine/Renderer/RenderTypes.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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

struct SceneData
{
    static constexpr std::uint32_t
        CurrentVersion = 1;

    std::uint32_t version =
        CurrentVersion;

    std::vector<SerializedEntity>
        entities;
};