#pragma once

#include <memory>
#include <string>

struct AnimationClipData;

class AnimationClipSerializer
{
public:
    static std::unique_ptr<AnimationClipData>
        Load(
            const std::wstring& path);
};