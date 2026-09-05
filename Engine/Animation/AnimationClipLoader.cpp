#include "AnimationClipLoader.h"

#include "AnimationClip.h"
#include "Engine/Animation/AnimationClipData.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/AnimationClipSerializer.h"

#include <memory>

std::unique_ptr<AnimationClip>
AnimationClipLoader::Load(
    const std::wstring& path,
    ResourceManager& resources)
{
    auto data =
        AnimationClipSerializer::Load(
            path);

    if (!data)
    {
        ANIMATIONCLIP_DEBUG_LOG(
            "Failed to deserialize "
            "AnimationClip.");

        return nullptr;
    }

    Texture* texture =
        resources.LoadTexture(
            data->texturePath);

    if (!texture)
    {
        ANIMATIONCLIP_DEBUG_LOG(
            "Failed to load "
            "AnimationClip texture.");

        return nullptr;
    }

    auto clip =
        std::make_unique<
        AnimationClip>();

    clip->SetTexture(
        texture);

    clip->SetLooping(
        data->looping);

    for (const
        AnimationClipFrameData& frame :
        data->frames)
    {
        if (!clip->AddFrame(
            frame.uv,
            frame.duration))
        {
            ANIMATIONCLIP_DEBUG_LOG(
                "Failed to create "
                "AnimationClip frame.");

            return nullptr;
        }
    }

    if (!clip->IsValid())
    {
        ANIMATIONCLIP_DEBUG_LOG(
            "AnimationClip is invalid.");

        return nullptr;
    }

    return clip;
}