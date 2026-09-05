#include "SceneHierarchyPanel.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Scene.h"

#include <imgui.h>

#include <cstdio>

SceneHierarchyPanel::SelectionResult
SceneHierarchyPanel::DrawContents(
    const Scene* scene,
    EntityHandle selectedEntityHandle
)
{
    SelectionResult result{};

    if (!scene)
    {
        ImGui::TextDisabled(
            "No active scene."
        );

        if (selectedEntityHandle.IsValid())
        {
            result.changed = true;
            result.handle =
                EntityHandle{};
        }

        return result;
    }

    //
    // Current frame에서 실제로 표시할 selection.
    //
    // Clear가 눌리면 EditorSystem에 commit되기 전이라도
    // 이 frame의 Hierarchy에서는 즉시 selection을
    // 해제해서 표시한다.
    //

    EntityHandle effectiveSelection =
        selectedEntityHandle;

    const bool hasSelection =
        effectiveSelection.IsValid();

    ImGui::BeginDisabled(
        !hasSelection
    );

    const bool clearRequested =
        ImGui::Button(
            "Clear Selection"
        );

    ImGui::EndDisabled();

    if (clearRequested)
    {
        effectiveSelection =
            EntityHandle{};

        result.changed = true;
        result.handle =
            EntityHandle{};
    }

    ImGui::Separator();

    const std::size_t entityCount =
        scene->GetEntityCount();

    bool hasVisibleEntity = false;

    for (std::size_t index = 0;
        index < entityCount;
        ++index)
    {
        const Entity* entity =
            scene->GetEntityAt(
                index
            );

        if (!entity)
        {
            continue;
        }

        const EntityHandle handle =
            entity->GetHandle();

        if (!scene->IsEntityAlive(
            handle
        ))
        {
            continue;
        }

        hasVisibleEntity = true;

        const bool selected =
            effectiveSelection ==
            handle;

        char label[96]{};

        std::snprintf(
            label,
            sizeof(label),
            "Entity [Handle %llu]",
            static_cast<
            unsigned long long
            >(
                handle.value
                )
        );

        const bool clicked =
            ImGui::Selectable(
                label,
                selected
            );

        if (clicked)
        {
            effectiveSelection =
                handle;

            result.changed = true;
            result.handle =
                handle;
        }
    }

    if (!hasVisibleEntity)
    {
        ImGui::TextDisabled(
            "Scene has no entities."
        );
    }

    return result;
}