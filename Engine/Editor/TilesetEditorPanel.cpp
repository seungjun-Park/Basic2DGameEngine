#include "TilesetEditorPanel.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"

#include <Windows.h>

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <limits>

bool TilesetEditorPanel::OpenTexture(
    const std::wstring& texturePath,
    ResourceManager& resourceManager
)
{
    if (texturePath.empty())
    {
        return false;
    }

    Texture* texture =
        resourceManager.LoadTexture(
            texturePath
        );

    if (!texture)
    {
        return false;
    }

    if (texture->GetWidth() <= 0 ||
        texture->GetHeight() <= 0)
    {
        return false;
    }

    //
    // Commit only after the texture has
    // been fully validated.
    //

    m_texturePath =
        texturePath;

    m_texture =
        texture;

    //
    // Start from a valid 1x1 tileset.
    // The user can then enter the actual
    // slicing dimensions.
    //

    m_tileWidth =
        texture->GetWidth();

    m_tileHeight =
        texture->GetHeight();

    m_margin = 0;
    m_spacing = 0;

    m_previewScale = 1.0f;

    m_fitPreviewToWidth = true;

    RecalculateGrid();

    return true;
}

void TilesetEditorPanel::Close() noexcept
{
    m_texturePath.clear();

    m_texture =
        nullptr;

    m_tileWidth = 0;
    m_tileHeight = 0;

    m_columns = 0;
    m_rows = 0;

    m_margin = 0;
    m_spacing = 0;

    m_previewScale = 1.0f;

    m_fitPreviewToWidth = true;
}

void TilesetEditorPanel::DrawContents()
{
    if (!m_texture)
    {
        ImGui::TextDisabled(
            "No texture is open."
        );

        ImGui::TextDisabled(
            "Open a Texture from the Assets tab."
        );

        return;
    }

    const std::string texturePath =
        ToUtf8(
            m_texturePath
        );

    ImGui::TextUnformatted(
        "Texture"
    );

    ImGui::TextWrapped(
        "%s",
        texturePath.empty()
        ? "<invalid path>"
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

    bool layoutChanged = false;

    if (ImGui::InputInt(
        "Tile Width",
        &m_tileWidth
    ))
    {
        m_tileWidth =
            std::max(
                1,
                m_tileWidth
            );

        layoutChanged = true;
    }

    if (ImGui::InputInt(
        "Tile Height",
        &m_tileHeight
    ))
    {
        m_tileHeight =
            std::max(
                1,
                m_tileHeight
            );

        layoutChanged = true;
    }

    if (ImGui::InputInt(
        "Margin",
        &m_margin
    ))
    {
        m_margin =
            std::max(
                0,
                m_margin
            );

        layoutChanged = true;
    }

    if (ImGui::InputInt(
        "Spacing",
        &m_spacing
    ))
    {
        m_spacing =
            std::max(
                0,
                m_spacing
            );

        layoutChanged = true;
    }

    if (layoutChanged)
    {
        RecalculateGrid();
    }

    ImGui::Separator();

    ImGui::Text(
        "Columns: %d",
        m_columns
    );

    ImGui::SameLine();

    ImGui::Text(
        "Rows: %d",
        m_rows
    );

    ImGui::SameLine();

    const std::int64_t tileCount =
        static_cast<std::int64_t>(
            m_columns
            ) *
        static_cast<std::int64_t>(
            m_rows
            );

    ImGui::Text(
        "Tiles: %lld",
        static_cast<long long>(
            tileCount
            )
    );

    if (m_columns <= 0 ||
        m_rows <= 0)
    {
        ImGui::TextDisabled(
            "The current slice does not fit "
            "inside the texture."
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

bool TilesetEditorPanel::IsOpen() const noexcept
{
    return
        m_texture != nullptr;
}

void TilesetEditorPanel::RecalculateGrid() noexcept
{
    m_columns = 0;
    m_rows = 0;

    if (!m_texture)
    {
        return;
    }

    if (m_tileWidth <= 0 ||
        m_tileHeight <= 0 ||
        m_margin < 0 ||
        m_spacing < 0)
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
            m_tileWidth
            );

    const std::int64_t tileHeight =
        static_cast<std::int64_t>(
            m_tileHeight
            );

    const std::int64_t margin =
        static_cast<std::int64_t>(
            m_margin
            );

    const std::int64_t spacing =
        static_cast<std::int64_t>(
            m_spacing
            );

    //
    // Match the current Tileset runtime
    // layout contract exactly:
    //
    // pixelX =
    //     margin +
    //     column * (tileWidth + spacing)
    //
    // There is no required trailing margin.
    //

    if (textureWidth <
        margin + tileWidth)
    {
        return;
    }

    if (textureHeight <
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

    m_columns =
        static_cast<int>(
            columns
            );

    m_rows =
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

    if (m_columns <= 0 ||
        m_rows <= 0)
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
            m_tileWidth +
            m_spacing
            );

    const float strideY =
        static_cast<float>(
            m_tileHeight +
            m_spacing
            );

    for (int row = 0;
        row < m_rows;
        ++row)
    {
        for (int column = 0;
            column < m_columns;
            ++column)
        {
            const float pixelX =
                static_cast<float>(
                    m_margin
                    ) +
                static_cast<float>(
                    column
                    ) *
                strideX;

            const float pixelY =
                static_cast<float>(
                    m_margin
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
                        m_tileWidth
                    ) *
                    scaleX,

                tileMin.y +
                    static_cast<float>(
                        m_tileHeight
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

std::string
TilesetEditorPanel::ToUtf8(
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