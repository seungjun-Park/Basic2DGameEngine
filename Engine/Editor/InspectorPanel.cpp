#include "InspectorPanel.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Animation/Animator.h"
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

bool InspectorPanel::DrawContents(
    Entity& entity,
    const AssetDatabase& assetDatabase,
    const std::wstring& selectedAssetPath,
    ResourceManager& resourceManager,
    bool editable
)
{
    bool changed =
        false;

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

    const bool activeChanged =
        ImGui::Checkbox(
            "Active",
            &entity.active
        );

    ImGui::EndDisabled();

    if (activeChanged)
    {
        changed =
            true;
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader(
        "Transform",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        changed =
            DrawTransform(
                entity,
                editable
            ) ||
            changed;
    }

    if (ImGui::CollapsingHeader(
        "Sprite",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        changed =
            DrawSprite(
                entity,
                editable
            ) ||
            changed;
    }

    if (ImGui::CollapsingHeader(
        "Asset Assignment",
        ImGuiTreeNodeFlags_DefaultOpen
    ))
    {
        changed =
            DrawAssetAssignment(
                entity,
                assetDatabase,
                selectedAssetPath,
                resourceManager,
                editable
            ) ||
            changed;
    }

    return
        changed;
}

bool InspectorPanel::DrawTransform(
    Entity& entity,
    bool editable
)
{
    bool changed =
        false;

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

        changed =
            true;
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

        changed =
            true;
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

        changed =
            true;
    }

    ImGui::EndDisabled();

    return
        changed;
}

bool InspectorPanel::DrawSprite(
    Entity& entity,
    bool editable
)
{
    bool changed =
        false;

    ImGui::BeginDisabled(
        !editable
    );

    if (ImGui::Checkbox(
        "Visible",
        &entity.sprite.visible
    ))
    {
        changed =
            true;
    }

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

    int currentLayer =
        0;

    for (int index = 0;
        index < 6;
        ++index)
    {
        if (entity.sprite.layer ==
            layers[index])
        {
            currentLayer =
                index;

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
            layers[
                currentLayer
            ];

        changed =
            true;
    }

    if (ImGui::DragFloat(
        "Z Index",
        &entity.sprite.zIndex,
        0.1f
    ))
    {
        changed =
            true;
    }

    if (ImGui::Checkbox(
        "Use Y Sort",
        &entity.sprite.useYSort
    ))
    {
        changed =
            true;
    }

    constexpr BlendMode blendModes[]
    {
        BlendMode::Opaque,
        BlendMode::Alpha
    };

    constexpr const char*
        blendModeNames[]
    {
        "Opaque",
        "Alpha"
    };

    int currentBlendMode =
        entity.sprite.blendMode ==
        BlendMode::Alpha
        ? 1
        : 0;

    if (ImGui::Combo(
        "Blend Mode",
        &currentBlendMode,
        blendModeNames,
        2
    ))
    {
        entity.sprite.blendMode =
            blendModes[
                currentBlendMode
            ];

        changed =
            true;
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

        changed =
            true;
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

        changed =
            true;
    }

    const bool resetUvRequested =
        ImGui::Button(
            "Reset UV"
        );

    ImGui::EndDisabled();

    if (resetUvRequested)
    {
        entity.sprite.uv =
        {
            0.0f,
            0.0f,
            1.0f,
            1.0f
        };

        changed =
            true;
    }

    return
        changed;
}

bool InspectorPanel::DrawAssetAssignment(
    Entity& entity,
    const AssetDatabase& assetDatabase,
    const std::wstring& selectedAssetPath,
    ResourceManager& resourceManager,
    bool editable
)
{
    bool changed =
        false;

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

        return false;
    }

    ImGui::Text(
        "Selected Type: %s",
        GetAssetTypeName(
            selectedAsset->type
        )
    );

    //
    // Texture assignment
    //

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

        if (texture &&
            entity.sprite.texture !=
            texture)
        {
            entity.sprite.texture =
                texture;

            changed =
                true;
        }
    }

    //
    // Animation assignment
    //

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

            if (entity.animator->
                Play(
                    *clip,
                    true
                ))
            {
                //
                // Play() applies the first AnimationFrame
                // to Sprite immediately, so serialized
                // Sprite state has changed.
                //
                changed =
                    true;
            }
        }
    }

    ImGui::TextDisabled(
        "SceneData V2 persists the resulting Sprite "
        "state. Generic per-entity Animator bindings "
        "are not serialized yet."
    );

    return
        changed;
}