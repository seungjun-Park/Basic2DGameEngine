#pragma once

#include <string>

class ResourceManager;
class Texture;

class TilesetEditorPanel
{
public:
    bool OpenTexture(
        const std::wstring& texturePath,
        ResourceManager& resourceManager
    );

    void Close() noexcept;

    void DrawContents();

    [[nodiscard]]
    bool IsOpen() const noexcept;

private:
    void RecalculateGrid() noexcept;

    void DrawPreview();

    [[nodiscard]]
    static std::string ToUtf8(
        const std::wstring& value
    );

private:
    std::wstring m_texturePath;

    Texture* m_texture = nullptr;

    int m_tileWidth = 0;
    int m_tileHeight = 0;

    int m_columns = 0;
    int m_rows = 0;

    int m_margin = 0;
    int m_spacing = 0;

    float m_previewScale = 1.0f;

    bool m_fitPreviewToWidth = true;
};
