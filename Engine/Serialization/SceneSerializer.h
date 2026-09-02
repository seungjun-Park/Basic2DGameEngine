#pragma once

#include <memory>
#include <string>

#include "SceneData.h"

class SceneSerializer
{
public:
    static bool Save(
        const SceneData& scene,
        const std::wstring& path
    );

    static std::unique_ptr<SceneData>
        Load(
            const std::wstring& path
        );
};