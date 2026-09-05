#pragma once

#include <memory>
#include <string>

struct AnimationClipData;

class AnimationClipSerializer
{
public:
    static bool Save(
        const AnimationClipData& data,
        const std::wstring& path);

    static std::unique_ptr<
        AnimationClipData>
        Load(
            const std::wstring& path);

    [[nodiscard]]
    static bool Validate(
        const AnimationClipData& data)
        noexcept;
};