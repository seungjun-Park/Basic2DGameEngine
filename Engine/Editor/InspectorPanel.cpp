#include "InspectorPanel.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Editor/AssetDatabase.h"
#include "Engine/Physics/PhysicsBody.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Scene/Entity.h"

#include <imgui.h>

#include <memory>

namespace
{
    const char* GetAssetTypeName(
        AssetType type
    ) noexcept
    {
        switch (type)
        {
        case AssetType::Texture:
            return "Texture";

        case AssetType::AnimationClip:
            return "Animation Clip";

        case AssetType::AudioClip:
            return "Audio Clip";

        case AssetType::Tileset:
            return "Tileset";

        case AssetType::TileMap:
            return "TileMap";

        case AssetType::Scene:
            return "Scene";

        case AssetType::Shader:
            return "Shader";

        case AssetType::Unknown:
        default:
            return "Unknown";
        }
    }
}

void InspectorPanel::DrawContents(
    Entity& entity,
    const AssetDatabase& assetDatabase,
    const std::wstring& selectedAssetPath,
    ResourceManager& resourceManager,
    bool editable
)
{
    ImGui::Text(
        "Handle: %llu",
        static_cast<unsigned long long>(
            entity.GetHandle().value
            )
    );

    if (!editable)
    {
        ImGui::TextDisabled(
            "Editing is disabled in Play Mode."
        );
    }

    ImGui::BeginDisabled(
        !editable
    );

    ImGui::Checkbox(
        "Active",
        &entity.active
    );

    ImGui::EndDisabled();

    ImGui::Separator();

    if (ImGui::CollapsingHeader(
        "Transform",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        DrawTransform(
            entity,
            editable
        );
    }

    if (ImGui::CollapsingHeader(
        "Sprite",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        DrawSprite(
            entity,
            editable
        );
    }

    if (ImGui::CollapsingHeader(
        "Asset Assignment",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        DrawAssetAssignment(
            entity,
            assetDatabase,
            selectedAssetPath,
            resourceManager,
            editable
        );
    }
}

void InspectorPanel::DrawTransform(
    Entity& entity,
    bool editable
)
{
    ImGui::BeginDisabled(
        !editable
    );

    float position[2]
    {
        entity.transform.position.x,
        entity.transform.position.y
    };

    const bool positionChanged =
        ImGui::DragFloat2(
            "Position",
            position,
            1.0f
        );

    if (positionChanged)
    {
        entity.transform.position.x =
            position[0];

        entity.transform.position.y =
            position[1];

        if (entity.physicsBody &&
            entity.physicsBody->IsValid())
        {
            entity.physicsBody->
                SetPosition(
                    position[0],
                    position[1]
                );
        }
    }

    float scale[2]
    {
        entity.transform.scale.x,
        entity.transform.scale.y
    };

    if (ImGui::DragFloat2(
        "Scale",
        scale,
        1.0f
    ))
    {
        entity.transform.scale.x =
            scale[0];

        entity.transform.scale.y =
            scale[1];
    }

    float rotation =
        entity.transform.rotation;

    if (ImGui::DragFloat(
        "Rotation",
        &rotation,
        0.01f
    ))
    {
        entity.transform.rotation =
            rotation;

        if (entity.physicsBody &&
            entity.physicsBody->IsValid())
        {
            entity.physicsBody->
                SetRotation(
                    rotation
                );
        }
    }

    ImGui::EndDisabled();
}

void InspectorPanel::DrawSprite(
    Entity& entity,
    bool editable
)
{
    ImGui::BeginDisabled(
        !editable
    );

    ImGui::Checkbox(
        "Visible",
        &entity.sprite.visible
    );

    constexpr RenderLayer layers[]
    {
        RenderLayer::Background,
        RenderLayer::World,
        RenderLayer::Effect,
        RenderLayer::Foreground,
        RenderLayer::UI,
        RenderLayer::Debug
    };

    constexpr const char* layerNames[]
    {
        "Background",
        "World",
        "Effect",
        "Foreground",
        "UI",
        "Debug"
    };

    int currentLayer = 0;

    for (int index = 0;
        index < 6;
        ++index)
    {
        if (entity.sprite.layer ==
            layers[index])
        {
            currentLayer = index;
            break;
        }
    }

    if (ImGui::Combo(
        "Render Layer",
        &currentLayer,
        layerNames,
        6
    ))
    {
        entity.sprite.layer =
            layers[currentLayer];
    }

    ImGui::DragFloat(
        "Z Index",
        &entity.sprite.zIndex,
        0.1f
    );

    ImGui::Checkbox(
        "Use Y Sort",
        &entity.sprite.useYSort
    );

    constexpr BlendMode blendModes[]
    {
        BlendMode::Opaque,
        BlendMode::Alpha
    };

    constexpr const char* blendModeNames[]
    {
        "Opaque",
        "Alpha"
    };

    int currentBlendMode = 0;

    if (entity.sprite.blendMode ==
        BlendMode::Alpha)
    {
        currentBlendMode = 1;
    }

    if (ImGui::Combo(
        "Blend Mode",
        &currentBlendMode,
        blendModeNames,
        2
    ))
    {
        entity.sprite.blendMode =
            blendModes[currentBlendMode];
    }

    float uvMin[2]
    {
        entity.sprite.uv.u0,
        entity.sprite.uv.v0
    };

    if (ImGui::DragFloat2(
        "UV Min",
        uvMin,
        0.01f
    ))
    {
        entity.sprite.uv.u0 =
            uvMin[0];

        entity.sprite.uv.v0 =
            uvMin[1];
    }

    float uvMax[2]
    {
        entity.sprite.uv.u1,
        entity.sprite.uv.v1
    };

    if (ImGui::DragFloat2(
        "UV Max",
        uvMax,
        0.01f
    ))
    {
        entity.sprite.uv.u1 =
            uvMax[0];

        entity.sprite.uv.v1 =
            uvMax[1];
    }

    if (ImGui::Button(
        "Reset UV"
    ))
    {
        entity.sprite.uv =
        {
            0.0f,
            0.0f,
            1.0f,
            1.0f
        };
    }

    ImGui::EndDisabled();
}

void InspectorPanel::DrawAssetAssignment(
    Entity& entity,
    const AssetDatabase& assetDatabase,
    const std::wstring& selectedAssetPath,
    ResourceManager& resourceManager,
    bool editable
)
{
    const AssetRecord* selectedAsset =
        nullptr;

    if (!selectedAssetPath.empty())
    {
        selectedAsset =
            assetDatabase.FindAsset(
                selectedAssetPath
            );
    }

    if (!selectedAsset)
    {
        ImGui::TextDisabled(
            "No asset selected."
        );

        return;
    }

    ImGui::Text(
        "Selected Type: %s",
        GetAssetTypeName(
            selectedAsset->type
        )
    );

    const bool canAssignTexture =
        editable &&
        selectedAsset->type ==
        AssetType::Texture;

    ImGui::BeginDisabled(
        !canAssignTexture
    );

    const bool assignTextureRequested =
        ImGui::Button(
            "Assign Texture"
        );

    ImGui::EndDisabled();

    if (assignTextureRequested)
    {
        Texture* texture =
            resourceManager.LoadTexture(
                selectedAsset->path
            );

        if (texture)
        {
            entity.sprite.texture =
                texture;
        }
    }

    const bool canAssignAnimation =
        editable &&
        selectedAsset->type ==
        AssetType::AnimationClip;

    ImGui::BeginDisabled(
        !canAssignAnimation
    );

    const bool assignAnimationRequested =
        ImGui::Button(
            "Assign Animation"
        );

    ImGui::EndDisabled();

    if (assignAnimationRequested)
    {
        AnimationClip* clip =
            resourceManager.
            LoadAnimationClip(
                selectedAsset->path
            );

        if (clip)
        {
            if (!entity.animator)
            {
                entity.animator =
                    std::make_unique<
                    Animator
                    >(
                        entity.sprite
                    );
            }

            entity.animator->Play(
                *clip,
                true
            );
        }
    }
}