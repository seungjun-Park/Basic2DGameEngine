#include "TilesetEditorPanel.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TilesetSerializer.h"

#include <Windows.h>

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

namespace
{
    bool CalculateRequiredExtent(
        int tileSize,
        int count,
        int margin,
        int spacing,
        std::int64_t& outExtent
    ) noexcept
    {
        outExtent = 0;

        if (tileSize <= 0 ||
            count <= 0 ||
            margin < 0 ||
            spacing < 0)
        {
            return false;
        }

        const std::int64_t tile =
            static_cast<std::int64_t>(
                tileSize
                );

        const std::int64_t itemCount =
            static_cast<std::int64_t>(
                count
                );

        const std::int64_t outerMargin =
            static_cast<std::int64_t>(
                margin
                );

        const std::int64_t gap =
            static_cast<std::int64_t>(
                spacing
                );

        const std::int64_t stride =
            tile +
            gap;

        const std::int64_t stepCount =
            itemCount -
            1;

        constexpr std::int64_t maximum =
            std::numeric_limits<
            std::int64_t
            >::max();

        if (outerMargin >
            maximum - tile)
        {
            return false;
        }

        const std::int64_t base =
            outerMargin +
            tile;

        if (stepCount > 0)
        {
            if (stride <= 0)
            {
                return false;
            }

            if (stepCount >
                (maximum - base) /
                stride)
            {
                return false;
            }
        }

        outExtent =
            base +
            stepCount *
            stride;

        return true;
    }
}

bool TilesetEditorPanel::Open(
    const std::wstring& path,
    ResourceManager& resourceManager
)
{
    m_lastOpenFailed =
        false;

    m_openBlockedByDirty =
        false;

    if (path.empty())
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    if (m_isOpen &&
        m_hasSavedDocument &&
        path == m_documentPath)
    {
        return true;
    }

    if (m_isOpen &&
        IsDirty())
    {
        m_openBlockedByDirty =
            true;

        return false;
    }

    auto data =
        TilesetSerializer::Load(
            path
        );

    if (!data)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    Texture* texture =
        resourceManager.LoadTexture(
            data->texturePath
        );

    if (!texture)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    if (texture->GetWidth() <= 0 ||
        texture->GetHeight() <= 0)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    std::int64_t requiredWidth = 0;
    std::int64_t requiredHeight = 0;

    if (!CalculateRequiredExtent(
        data->tileWidth,
        data->columns,
        data->margin,
        data->spacing,
        requiredWidth
    ) ||
        !CalculateRequiredExtent(
            data->tileHeight,
            data->rows,
            data->margin,
            data->spacing,
            requiredHeight
        ))
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    if (requiredWidth >
        static_cast<std::int64_t>(
            texture->GetWidth()
            ) ||
        requiredHeight >
        static_cast<std::int64_t>(
            texture->GetHeight()
            ))
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    m_savedData =
        *data;

    m_draftData =
        *data;

    m_documentPath =
        path;

    m_texture =
        texture;

    m_previewScale =
        1.0f;

    m_fitPreviewToWidth =
        true;

    m_isOpen =
        true;

    m_hasSavedDocument =
        true;

    m_lastOpenFailed =
        false;

    m_openBlockedByDirty =
        false;

    m_lastSaveSucceeded =
        false;

    m_lastSaveFailed =
        false;

    m_textEncodingError =
        false;

    SyncDocumentPathBuffer();

    return true;
}

bool TilesetEditorPanel::OpenTexture(
    const std::wstring& texturePath,
    ResourceManager& resourceManager
)
{
    m_lastOpenFailed =
        false;

    m_openBlockedByDirty =
        false;

    if (texturePath.empty())
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    //
    // 현재 열려 있는 동일한 unsaved slicer를
    // 다시 요청한 경우 draft를 폐기하지 않고
    // 기존 document를 그대로 사용한다.
    //
    if (m_isOpen &&
        !m_hasSavedDocument &&
        m_draftData.texturePath ==
        texturePath)
    {
        return true;
    }

    if (m_isOpen &&
        IsDirty())
    {
        m_openBlockedByDirty =
            true;

        return false;
    }

    Texture* texture =
        resourceManager.LoadTexture(
            texturePath
        );

    if (!texture)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    if (texture->GetWidth() <= 0 ||
        texture->GetHeight() <= 0)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    TilesetData data{};

    data.texturePath =
        texturePath;

    data.tileWidth =
        texture->GetWidth();

    data.tileHeight =
        texture->GetHeight();

    data.columns = 1;
    data.rows = 1;

    data.margin = 0;
    data.spacing = 0;

    m_savedData =
        data;

    m_draftData =
        data;

    m_documentPath.clear();

    m_documentPathBuffer.fill(
        '\0'
    );

    m_texture =
        texture;

    m_previewScale =
        1.0f;

    m_fitPreviewToWidth =
        true;

    m_isOpen =
        true;

    m_hasSavedDocument =
        false;

    m_lastOpenFailed =
        false;

    m_openBlockedByDirty =
        false;

    m_lastSaveSucceeded =
        false;

    m_lastSaveFailed =
        false;

    m_textEncodingError =
        false;

    return true;
}

void TilesetEditorPanel::Close()
noexcept
{
    m_savedData =
        TilesetData{};

    m_draftData =
        TilesetData{};

    m_documentPath.clear();

    m_documentPathBuffer.fill(
        '\0'
    );

    m_texture =
        nullptr;

    m_previewScale =
        1.0f;

    m_fitPreviewToWidth =
        true;

    m_isOpen =
        false;

    m_hasSavedDocument =
        false;

    m_lastOpenFailed =
        false;

    m_openBlockedByDirty =
        false;

    m_lastSaveSucceeded =
        false;

    m_lastSaveFailed =
        false;

    m_textEncodingError =
        false;
}

void TilesetEditorPanel::DrawContents()
{
    if (!m_isOpen ||
        !m_texture)
    {
        ImGui::TextDisabled(
            "No Tileset document is open."
        );

        ImGui::TextDisabled(
            "Open a Texture or Tileset "
            "from the Assets tab."
        );

        return;
    }

    //
    // Document
    //

    ImGui::TextUnformatted(
        "Tileset Document"
    );

    if (!m_hasSavedDocument)
    {
        ImGui::TextDisabled(
            "New Tileset"
        );

        if (ImGui::InputText(
            "Tileset Path",
            m_documentPathBuffer.data(),
            m_documentPathBuffer.size()
        ))
        {
            const std::string utf8
            {
                m_documentPathBuffer.data()
            };

            if (utf8.empty())
            {
                m_documentPath.clear();

                m_textEncodingError =
                    false;
            }
            else
            {
                const std::wstring wide =
                    Utf8ToWide(
                        utf8
                    );

                if (!wide.empty())
                {
                    m_documentPath =
                        wide;

                    m_textEncodingError =
                        false;
                }
                else
                {
                    m_textEncodingError =
                        true;
                }
            }
        }
    }
    else
    {
        const std::string documentPath =
            ToUtf8(
                m_documentPath
            );

        ImGui::TextWrapped(
            "%s",
            documentPath.empty()
            ? "<invalid document path>"
            : documentPath.c_str()
        );
    }

    //
    // Frame-local snapshots.
    //
    // 이후 Save/Revert가 state를 변경하더라도
    // BeginDisabled/EndDisabled 조건은 동일 frame
    // 안에서 변하지 않는다.
    //

    const bool dirty =
        IsDirty();

    const bool draftValid =
        TilesetSerializer::Validate(
            m_draftData
        ) &&
        ValidateDraftAgainstTexture();

    const bool saveAvailable =
        dirty &&
        draftValid &&
        !m_documentPath.empty() &&
        !m_textEncodingError;

    if (!m_hasSavedDocument)
    {
        ImGui::Text(
            "Status: Unsaved"
        );
    }
    else if (dirty)
    {
        ImGui::Text(
            "Status: Modified"
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Status: Saved"
        );
    }

    if (m_textEncodingError)
    {
        ImGui::TextDisabled(
            "Document path encoding is invalid."
        );
    }

    if (!draftValid)
    {
        ImGui::TextDisabled(
            "Current Tileset layout is invalid."
        );
    }

    if (m_openBlockedByDirty)
    {
        ImGui::TextDisabled(
            "Open blocked: save, revert, "
            "or discard this document first."
        );
    }

    if (m_lastOpenFailed)
    {
        ImGui::TextDisabled(
            "The requested Tileset "
            "could not be opened."
        );
    }

    if (m_lastSaveSucceeded)
    {
        ImGui::TextDisabled(
            "Save succeeded."
        );
    }

    if (m_lastSaveFailed)
    {
        ImGui::TextDisabled(
            "Save failed."
        );
    }

    //
    // Save
    //

    ImGui::BeginDisabled(
        !saveAvailable
    );

    const bool saveRequested =
        ImGui::Button(
            "Save"
        );

    ImGui::EndDisabled();

    if (saveRequested)
    {
        Save();
    }

    //
    // Revert
    //

    ImGui::SameLine();

    ImGui::BeginDisabled(
        !m_hasSavedDocument ||
        !dirty
    );

    const bool revertRequested =
        ImGui::Button(
            "Revert"
        );

    ImGui::EndDisabled();

    if (revertRequested)
    {
        Revert();
    }

    //
    // Close
    //

    ImGui::SameLine();

    ImGui::BeginDisabled(
        dirty
    );

    const bool closeRequested =
        ImGui::Button(
            "Close"
        );

    ImGui::EndDisabled();

    if (closeRequested)
    {
        Close();
        return;
    }

    //
    // Dirty document explicit discard.
    //

    ImGui::SameLine();

    ImGui::BeginDisabled(
        !dirty
    );

    const bool discardRequested =
        ImGui::Button(
            "Discard & Close"
        );

    ImGui::EndDisabled();

    if (discardRequested)
    {
        Close();
        return;
    }

    ImGui::Separator();

    //
    // Texture information
    //

    const std::string texturePath =
        ToUtf8(
            m_draftData.texturePath
        );

    ImGui::TextUnformatted(
        "Texture"
    );

    ImGui::TextWrapped(
        "%s",
        texturePath.empty()
        ? "<invalid texture path>"
        : texturePath.c_str()
    );

    ImGui::Text(
        "Texture Size: %d x %d",
        m_texture->GetWidth(),
        m_texture->GetHeight()
    );

    ImGui::Separator();

    //
    // Slice configuration
    //

    bool layoutChanged =
        false;

    if (ImGui::InputInt(
        "Tile Width",
        &m_draftData.tileWidth
    ))
    {
        m_draftData.tileWidth =
            std::max(
                1,
                m_draftData.tileWidth
            );

        layoutChanged =
            true;
    }

    if (ImGui::InputInt(
        "Tile Height",
        &m_draftData.tileHeight
    ))
    {
        m_draftData.tileHeight =
            std::max(
                1,
                m_draftData.tileHeight
            );

        layoutChanged =
            true;
    }

    if (ImGui::InputInt(
        "Margin",
        &m_draftData.margin
    ))
    {
        m_draftData.margin =
            std::max(
                0,
                m_draftData.margin
            );

        layoutChanged =
            true;
    }

    if (ImGui::InputInt(
        "Spacing",
        &m_draftData.spacing
    ))
    {
        m_draftData.spacing =
            std::max(
                0,
                m_draftData.spacing
            );

        layoutChanged =
            true;
    }

    if (layoutChanged)
    {
        RecalculateGrid();

        m_lastSaveSucceeded =
            false;

        m_lastSaveFailed =
            false;
    }

    ImGui::Separator();

    ImGui::Text(
        "Columns: %d",
        m_draftData.columns
    );

    ImGui::SameLine();

    ImGui::Text(
        "Rows: %d",
        m_draftData.rows
    );

    ImGui::SameLine();

    const std::int64_t tileCount =
        static_cast<std::int64_t>(
            m_draftData.columns
            ) *
        static_cast<std::int64_t>(
            m_draftData.rows
            );

    ImGui::Text(
        "Tiles: %lld",
        static_cast<long long>(
            tileCount
            )
    );

    if (m_draftData.columns <= 0 ||
        m_draftData.rows <= 0)
    {
        ImGui::TextDisabled(
            "The current slice does not "
            "fit inside the texture."
        );
    }

    ImGui::Separator();

    //
    // Preview controls
    //

    ImGui::Checkbox(
        "Fit Preview To Width",
        &m_fitPreviewToWidth
    );

    ImGui::BeginDisabled(
        m_fitPreviewToWidth
    );

    ImGui::SliderFloat(
        "Preview Scale",
        &m_previewScale,
        0.10f,
        4.00f,
        "%.2fx"
    );

    ImGui::EndDisabled();

    ImGui::Separator();

    DrawPreview();
}

bool TilesetEditorPanel::IsOpen()
const noexcept
{
    return
        m_isOpen;
}

bool TilesetEditorPanel::IsDirty()
const noexcept
{
    if (!m_isOpen)
    {
        return false;
    }

    if (!m_hasSavedDocument)
    {
        return true;
    }

    return
        !AreEqual(
            m_savedData,
            m_draftData
        );
}

const std::wstring&
TilesetEditorPanel::
GetDocumentPath() const noexcept
{
    return
        m_documentPath;
}

bool TilesetEditorPanel::Save()
{
    m_lastSaveSucceeded =
        false;

    m_lastSaveFailed =
        false;

    if (!m_isOpen ||
        !m_texture ||
        m_documentPath.empty() ||
        m_textEncodingError)
    {
        m_lastSaveFailed =
            true;

        return false;
    }

    if (!TilesetSerializer::Validate(
        m_draftData
    ) ||
        !ValidateDraftAgainstTexture())
    {
        m_lastSaveFailed =
            true;

        return false;
    }

    if (!TilesetSerializer::Save(
        m_draftData,
        m_documentPath
    ))
    {
        m_lastSaveFailed =
            true;

        return false;
    }

    m_savedData =
        m_draftData;

    m_hasSavedDocument =
        true;

    m_lastSaveSucceeded =
        true;

    m_lastSaveFailed =
        false;

    m_openBlockedByDirty =
        false;

    SyncDocumentPathBuffer();

    return true;
}

void TilesetEditorPanel::Revert()
{
    if (!m_isOpen ||
        !m_hasSavedDocument)
    {
        return;
    }

    //
    // 중요:
    // RecalculateGrid()를 호출하지 않는다.
    //
    // Serialized Tileset은 texture 전체를
    // 모두 사용하는 것이 아니라 일부 columns/rows만
    // 명시적으로 사용하는 것도 유효하므로 savedData를
    // 정확히 복원해야 한다.
    //

    m_draftData =
        m_savedData;

    m_lastSaveSucceeded =
        false;

    m_lastSaveFailed =
        false;

    m_openBlockedByDirty =
        false;

    m_textEncodingError =
        false;
}

void TilesetEditorPanel::
RecalculateGrid() noexcept
{
    m_draftData.columns = 0;
    m_draftData.rows = 0;

    if (!m_texture)
    {
        return;
    }

    if (m_draftData.tileWidth <= 0 ||
        m_draftData.tileHeight <= 0 ||
        m_draftData.margin < 0 ||
        m_draftData.spacing < 0)
    {
        return;
    }

    const std::int64_t textureWidth =
        static_cast<std::int64_t>(
            m_texture->GetWidth()
            );

    const std::int64_t textureHeight =
        static_cast<std::int64_t>(
            m_texture->GetHeight()
            );

    const std::int64_t tileWidth =
        static_cast<std::int64_t>(
            m_draftData.tileWidth
            );

    const std::int64_t tileHeight =
        static_cast<std::int64_t>(
            m_draftData.tileHeight
            );

    const std::int64_t margin =
        static_cast<std::int64_t>(
            m_draftData.margin
            );

    const std::int64_t spacing =
        static_cast<std::int64_t>(
            m_draftData.spacing
            );

    if (textureWidth <
        margin + tileWidth ||
        textureHeight <
        margin + tileHeight)
    {
        return;
    }

    const std::int64_t strideX =
        tileWidth +
        spacing;

    const std::int64_t strideY =
        tileHeight +
        spacing;

    if (strideX <= 0 ||
        strideY <= 0)
    {
        return;
    }

    const std::int64_t columns =
        1 +
        (
            textureWidth -
            margin -
            tileWidth
            ) /
        strideX;

    const std::int64_t rows =
        1 +
        (
            textureHeight -
            margin -
            tileHeight
            ) /
        strideY;

    if (columns <= 0 ||
        rows <= 0)
    {
        return;
    }

    if (columns >
        static_cast<std::int64_t>(
            std::numeric_limits<int>::max()
            ) ||
        rows >
        static_cast<std::int64_t>(
            std::numeric_limits<int>::max()
            ))
    {
        return;
    }

    m_draftData.columns =
        static_cast<int>(
            columns
            );

    m_draftData.rows =
        static_cast<int>(
            rows
            );
}

void TilesetEditorPanel::DrawPreview()
{
    if (!m_texture)
    {
        return;
    }

    ID3D11ShaderResourceView* srv =
        m_texture->GetSRV();

    if (!srv)
    {
        ImGui::TextDisabled(
            "Texture preview is unavailable."
        );

        return;
    }

    const float textureWidth =
        static_cast<float>(
            m_texture->GetWidth()
            );

    const float textureHeight =
        static_cast<float>(
            m_texture->GetHeight()
            );

    if (textureWidth <= 0.0f ||
        textureHeight <= 0.0f)
    {
        return;
    }

    float displayScale =
        m_previewScale;

    if (m_fitPreviewToWidth)
    {
        const float availableWidth =
            ImGui::GetContentRegionAvail().x;

        if (availableWidth > 0.0f)
        {
            displayScale =
                std::min(
                    1.0f,
                    availableWidth /
                    textureWidth
                );
        }
    }

    displayScale =
        std::max(
            0.01f,
            displayScale
        );

    const ImVec2 displaySize
    {
        textureWidth *
            displayScale,

        textureHeight *
            displayScale
    };

    ImGui::Image(
        reinterpret_cast<ImTextureID>(
            srv
            ),
        displaySize
    );

    if (m_draftData.columns <= 0 ||
        m_draftData.rows <= 0)
    {
        return;
    }

    const ImVec2 imageMin =
        ImGui::GetItemRectMin();

    const ImVec2 imageMax =
        ImGui::GetItemRectMax();

    const float imageWidth =
        imageMax.x -
        imageMin.x;

    const float imageHeight =
        imageMax.y -
        imageMin.y;

    if (imageWidth <= 0.0f ||
        imageHeight <= 0.0f)
    {
        return;
    }

    const float scaleX =
        imageWidth /
        textureWidth;

    const float scaleY =
        imageHeight /
        textureHeight;

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    if (!drawList)
    {
        return;
    }

    const ImU32 gridColor =
        ImGui::GetColorU32(
            ImGuiCol_Separator
        );

    const float strideX =
        static_cast<float>(
            m_draftData.tileWidth +
            m_draftData.spacing
            );

    const float strideY =
        static_cast<float>(
            m_draftData.tileHeight +
            m_draftData.spacing
            );

    for (int row = 0;
        row < m_draftData.rows;
        ++row)
    {
        for (int column = 0;
            column <
            m_draftData.columns;
            ++column)
        {
            const float pixelX =
                static_cast<float>(
                    m_draftData.margin
                    ) +
                static_cast<float>(
                    column
                    ) *
                strideX;

            const float pixelY =
                static_cast<float>(
                    m_draftData.margin
                    ) +
                static_cast<float>(
                    row
                    ) *
                strideY;

            const ImVec2 tileMin
            {
                imageMin.x +
                    pixelX *
                    scaleX,

                imageMin.y +
                    pixelY *
                    scaleY
            };

            const ImVec2 tileMax
            {
                tileMin.x +
                    static_cast<float>(
                        m_draftData.tileWidth
                    ) *
                    scaleX,

                tileMin.y +
                    static_cast<float>(
                        m_draftData.tileHeight
                    ) *
                    scaleY
            };

            drawList->AddRect(
                tileMin,
                tileMax,
                gridColor
            );
        }
    }
}

bool TilesetEditorPanel::
ValidateDraftAgainstTexture()
const noexcept
{
    if (!m_texture ||
        !TilesetSerializer::Validate(
            m_draftData
        ))
    {
        return false;
    }

    std::int64_t requiredWidth = 0;
    std::int64_t requiredHeight = 0;

    if (!CalculateRequiredExtent(
        m_draftData.tileWidth,
        m_draftData.columns,
        m_draftData.margin,
        m_draftData.spacing,
        requiredWidth
    ) ||
        !CalculateRequiredExtent(
            m_draftData.tileHeight,
            m_draftData.rows,
            m_draftData.margin,
            m_draftData.spacing,
            requiredHeight
        ))
    {
        return false;
    }

    return
        requiredWidth <=
        static_cast<std::int64_t>(
            m_texture->GetWidth()
            ) &&
        requiredHeight <=
        static_cast<std::int64_t>(
            m_texture->GetHeight()
            );
}

void TilesetEditorPanel::
SyncDocumentPathBuffer()
{
    m_documentPathBuffer.fill(
        '\0'
    );

    if (m_documentPath.empty())
    {
        return;
    }

    const std::string utf8 =
        ToUtf8(
            m_documentPath
        );

    if (utf8.empty())
    {
        m_textEncodingError =
            true;

        return;
    }

    //
    // Saved document에서는 path buffer가
    // editing에 사용되지 않으므로 buffer보다 긴
    // 정상 path라는 이유로 document 자체를 invalid
    // 처리하지 않는다.
    //
    if (utf8.size() >=
        m_documentPathBuffer.size())
    {
        m_textEncodingError =
            false;

        return;
    }

    std::copy(
        utf8.begin(),
        utf8.end(),
        m_documentPathBuffer.begin()
    );

    m_documentPathBuffer[
        utf8.size()
    ] = '\0';

    m_textEncodingError =
        false;
}

bool TilesetEditorPanel::AreEqual(
    const TilesetData& lhs,
    const TilesetData& rhs
) noexcept
{
    return
        lhs.texturePath ==
        rhs.texturePath &&
        lhs.tileWidth ==
        rhs.tileWidth &&
        lhs.tileHeight ==
        rhs.tileHeight &&
        lhs.columns ==
        rhs.columns &&
        lhs.rows ==
        rhs.rows &&
        lhs.margin ==
        rhs.margin &&
        lhs.spacing ==
        rhs.spacing;
}

std::string TilesetEditorPanel::ToUtf8(
    const std::wstring& value
)
{
    if (value.empty())
    {
        return {};
    }

    if (value.size() >
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()
            ))
    {
        return {};
    }

    const int sourceLength =
        static_cast<int>(
            value.size()
            );

    const int requiredSize =
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            nullptr,
            0,
            nullptr,
            nullptr
        );

    if (requiredSize <= 0)
    {
        return {};
    }

    std::string result(
        static_cast<std::size_t>(
            requiredSize
            ),
        '\0'
    );

    const int convertedSize =
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            result.data(),
            requiredSize,
            nullptr,
            nullptr
        );

    if (convertedSize !=
        requiredSize)
    {
        return {};
    }

    return result;
}

std::wstring TilesetEditorPanel::Utf8ToWide(
    const std::string& value
)
{
    if (value.empty())
    {
        return {};
    }

    if (value.size() >
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()
            ))
    {
        return {};
    }

    const int sourceLength =
        static_cast<int>(
            value.size()
            );

    const int requiredSize =
        ::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            nullptr,
            0
        );

    if (requiredSize <= 0)
    {
        return {};
    }

    std::wstring result(
        static_cast<std::size_t>(
            requiredSize
            ),
        L'\0'
    );

    const int convertedSize =
        ::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            sourceLength,
            result.data(),
            requiredSize
        );

    if (convertedSize !=
        requiredSize)
    {
        return {};
    }

    return result;
}