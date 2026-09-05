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

    std::wstring
        animationClipPath;
};

struct SerializedAnimationBinding
{
    std::string slot;

    std::wstring clipPath;
};

struct SerializedAudioBinding
{
    std::string slot;

    std::wstring clipPath;

    float volume = 1.0f;
};

struct SceneData
{
    static constexpr std::uint32_t
        LegacyVersion = 1;

    static constexpr std::uint32_t
        ResourceBindingVersion = 2;

    static constexpr std::uint32_t
        AudioBindingVersion = 3;

    //
    // Kept as a compatibility alias for code that
    // previously referred to PreviousVersion.
    //
    static constexpr std::uint32_t
        PreviousVersion =
        AudioBindingVersion;

    static constexpr std::uint32_t
        CurrentVersion = 4;

    std::uint32_t version =
        CurrentVersion;

    std::wstring tileMapPath;

    std::vector<SerializedAnimationBinding>
        animationBindings;

    std::vector<SerializedAudioBinding>
        audioBindings;

    std::vector<SerializedEntity>
        entities;
};