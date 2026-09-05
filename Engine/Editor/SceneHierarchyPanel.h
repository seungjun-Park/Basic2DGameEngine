#pragma once

#include "Engine/Scene/EntityHandle.h"

class Scene;

class SceneHierarchyPanel
{
public:
    struct SelectionResult
    {
        bool changed = false;

        EntityHandle handle{};
    };

    [[nodiscard]]
    SelectionResult DrawContents(
        const Scene* scene,
        EntityHandle selectedEntityHandle
    );
};