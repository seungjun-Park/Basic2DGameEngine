#pragma once

#include <string>

class AssetDatabase;
class Entity;
class ResourceManager;

class InspectorPanel
{
public:
    bool DrawContents(
        Entity& entity,
        const AssetDatabase& assetDatabase,
        const std::wstring& selectedAssetPath,
        ResourceManager& resourceManager,
        bool editable
    );

private:
    bool DrawTransform(
        Entity& entity,
        bool editable
    );

    bool DrawSprite(
        Entity& entity,
        bool editable
    );

    bool DrawAssetAssignment(
        Entity& entity,
        const AssetDatabase& assetDatabase,
        const std::wstring& selectedAssetPath,
        ResourceManager& resourceManager,
        bool editable
    );
};