#pragma once

#include <memory>
#include <string>

class AnimationClip;
class ResourceManager;

class AnimationClipLoader
{
public:
    static std::unique_ptr<AnimationClip>
        Load(
            const std::wstring& path,
            ResourceManager& resources
        );
};