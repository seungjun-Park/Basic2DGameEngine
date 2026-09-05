#pragma once

#include <string>

class AssetDatabase;
class Entity;
class ResourceManager;

class InspectorPanel
{
public:
    void DrawContents(
        Entity& entity,
        const AssetDatabase& assetDatabase,
        const std::wstring& selectedAssetPath,
        ResourceManager& resourceManager,
        bool editable
    );

private:
    void DrawTransform(
        Entity& entity,
        bool editable
    );

    void DrawSprite(
        Entity& entity,
        bool editable
    );

    void DrawAssetAssignment(
        Entity& entity,
        const AssetDatabase& assetDatabase,
        const std::wstring& selectedAssetPath,
        ResourceManager& resourceManager,
        bool editable
    );
};