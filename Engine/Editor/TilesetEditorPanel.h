#pragma once

#include "Engine/Tile/TilesetData.h"

#include <array>
#include <string>

class ResourceManager;
class Texture;

class TilesetEditorPanel
{
public:
    bool Open(
        const std::wstring& path,
        ResourceManager& resourceManager
    );

    bool OpenTexture(
        const std::wstring& texturePath,
        ResourceManager& resourceManager
    );

    void Close() noexcept;

    void DrawContents();

    [[nodiscard]]
    bool IsOpen() const noexcept;

    [[nodiscard]]
    bool IsDirty() const noexcept;

    [[nodiscard]]
    const std::wstring&
        GetDocumentPath() const noexcept;

private:
    bool Save();

    void Revert();

    void RecalculateGrid() noexcept;

    void DrawPreview();

    [[nodiscard]]
    bool ValidateDraftAgainstTexture()
        const noexcept;

    void SyncDocumentPathBuffer();

    [[nodiscard]]
    static bool AreEqual(
        const TilesetData& lhs,
        const TilesetData& rhs
    ) noexcept;

    static std::string ToUtf8(
        const std::wstring& value
    );

    static std::wstring Utf8ToWide(
        const std::string& value
    );

private:
    TilesetData m_savedData{};
    TilesetData m_draftData{};

    std::wstring m_documentPath;

    std::array<char, 2048>
        m_documentPathBuffer{};

    Texture* m_texture =
        nullptr;

    float m_previewScale =
        1.0f;

    bool m_fitPreviewToWidth =
        true;

    bool m_isOpen =
        false;

    bool m_hasSavedDocument =
        false;

    bool m_lastOpenFailed =
        false;

    bool m_openBlockedByDirty =
        false;

    bool m_lastSaveSucceeded =
        false;

    bool m_lastSaveFailed =
        false;

    bool m_textEncodingError =
        false;
};