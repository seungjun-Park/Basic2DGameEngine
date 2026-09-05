#pragma once

#include "Engine/Graphics/UVRect.h"

#include <string>
#include <vector>

struct AnimationClipFrameData
{
    UVRect uv{};

    float duration =
        0.1f;
};

struct AnimationClipData
{
    std::wstring texturePath;

    std::vector<AnimationClipFrameData>
        frames;

    bool looping = true;
};