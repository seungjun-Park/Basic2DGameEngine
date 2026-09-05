#include "TilePalettePanel.h"

#include "Engine/Editor/AssetDatabase.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TilesetSerializer.h"

#include <Windows.h>

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

bool TilePalettePanel::OpenTileset(
    const std::wstring& path,
    ResourceManager& resourceManager
)
{
    m_lastOpenFailed =
        false;

    if (path.empty())
    {
        m_lastOpenFailed =
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

    if (!ValidateAgainstTexture(
        *data,
        *texture
    ))
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    //
    // Commit only after complete validation.
    //

    m_tilesetPath =
        path;

    m_tilesetData =
        *data;

    m_texture =
        texture;

    m_selectedTileId =
        InvalidTileId;

    m_lastOpenFailed =
        false;

    return true;
}

void TilePalettePanel::Close() noexcept
{
    m_tilesetPath.clear();

    m_tilesetData =
        TilesetData{};

    m_texture =
        nullptr;

    m_selectedTileId =
        InvalidTileId;

    m_thumbnailSize =
        64.0f;

    m_lastOpenFailed =
        false;
}

void TilePalettePanel::DrawContents(
    const AssetDatabase& assetDatabase,
    const std::wstring& selectedAssetPath,
    ResourceManager& resourceManager
)
{
    //
    // Selected Asset
    //

    const AssetRecord* selectedAsset =
        nullptr;

    if (!selectedAssetPath.empty())
    {
        selectedAsset =
            assetDatabase.FindAsset(
                selectedAssetPath
            );
    }

    const bool canLoadSelected =
        selectedAsset &&
        selectedAsset->type ==
        AssetType::Tileset;

    ImGui::BeginDisabled(
        !canLoadSelected
    );

    const bool loadSelectedRequested =
        ImGui::Button(
            "Load Selected Tileset"
        );

    ImGui::EndDisabled();

    if (loadSelectedRequested &&
        selectedAsset)
    {
        OpenTileset(
            selectedAsset->path,
            resourceManager
        );
    }

    ImGui::SameLine();

    const bool canReload =
        !m_tilesetPath.empty();

    ImGui::BeginDisabled(
        !canReload
    );

    const bool reloadRequested =
        ImGui::Button(
            "Reload"
        );

    ImGui::EndDisabled();

    if (reloadRequested)
    {
        OpenTileset(
            m_tilesetPath,
            resourceManager
        );
    }

    ImGui::SameLine();

    const bool canClose =
        IsOpen();

    ImGui::BeginDisabled(
        !canClose
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

    if (m_lastOpenFailed)
    {
        ImGui::TextDisabled(
            "Failed to load Tileset."
        );
    }

    if (!IsOpen())
    {
        ImGui::Separator();

        ImGui::TextDisabled(
            "No Tileset is loaded."
        );

        ImGui::TextDisabled(
            "Select a Tileset asset in the "
            "Assets tab, then press "
            "\"Load Selected Tileset\"."
        );

        return;
    }

    ImGui::Separator();

    //
    // Tileset information
    //

    const std::string tilesetPath =
        ToUtf8(
            m_tilesetPath
        );

    const std::string texturePath =
        ToUtf8(
            m_tilesetData.texturePath
        );

    ImGui::TextUnformatted(
        "Tileset"
    );

    ImGui::TextWrapped(
        "%s",
        tilesetPath.empty()
        ? "<invalid path>"
        : tilesetPath.c_str()
    );

    ImGui::Text(
        "Grid: %d x %d",
        m_tilesetData.columns,
        m_tilesetData.rows
    );

    ImGui::Text(
        "Tile Size: %d x %d",
        m_tilesetData.tileWidth,
        m_tilesetData.tileHeight
    );

    ImGui::TextWrapped(
        "Texture: %s",
        texturePath.empty()
        ? "<invalid texture path>"
        : texturePath.c_str()
    );

    ImGui::Separator();

    //
    // Current selection
    //
    // TileId 0 is deliberately used as the
    // "eraser / empty" selection.
    //

    if (m_selectedTileId ==
        InvalidTileId)
    {
        ImGui::Text(
            "Selected Tile: Eraser / Empty (0)"
        );
    }
    else
    {
        ImGui::Text(
            "Selected Tile: %u",
            static_cast<unsigned int>(
                m_selectedTileId
                )
        );
    }

    const bool eraserSelected =
        m_selectedTileId ==
        InvalidTileId;

    if (ImGui::Selectable(
        "Eraser / Empty [TileId 0]",
        eraserSelected
    ))
    {
        m_selectedTileId =
            InvalidTileId;
    }

    ImGui::Separator();

    ImGui::SliderFloat(
        "Thumbnail Size",
        &m_thumbnailSize,
        24.0f,
        128.0f,
        "%.0f px"
    );

    m_thumbnailSize =
        std::clamp(
            m_thumbnailSize,
            24.0f,
            128.0f
        );

    ImGui::Separator();

    DrawPalette();
}

bool TilePalettePanel::IsOpen()
const noexcept
{
    return
        m_texture != nullptr &&
        !m_tilesetPath.empty();
}

TileId
TilePalettePanel::GetSelectedTileId()
const noexcept
{
    return
        m_selectedTileId;
}

bool TilePalettePanel::HasSelectedTile()
const noexcept
{
    return
        m_selectedTileId !=
        InvalidTileId;
}

const std::wstring&
TilePalettePanel::GetTilesetPath()
const noexcept
{
    return
        m_tilesetPath;
}

void TilePalettePanel::DrawPalette()
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

    const std::uint64_t tileCount =
        GetTileCount();

    if (tileCount == 0)
    {
        ImGui::TextDisabled(
            "Tileset contains no tiles."
        );

        return;
    }

    const float tileWidth =
        static_cast<float>(
            m_tilesetData.tileWidth
            );

    const float tileHeight =
        static_cast<float>(
            m_tilesetData.tileHeight
            );

    if (tileWidth <= 0.0f ||
        tileHeight <= 0.0f)
    {
        return;
    }

    //
    // Preserve tile aspect ratio.
    //

    float previewWidth =
        m_thumbnailSize;

    float previewHeight =
        m_thumbnailSize *
        (
            tileHeight /
            tileWidth
            );

    if (previewHeight >
        m_thumbnailSize * 2.0f)
    {
        previewHeight =
            m_thumbnailSize * 2.0f;

        previewWidth =
            previewHeight *
            (
                tileWidth /
                tileHeight
                );
    }

    previewWidth =
        std::max(
            1.0f,
            previewWidth
        );

    previewHeight =
        std::max(
            1.0f,
            previewHeight
        );

    const float availableWidth =
        ImGui::GetContentRegionAvail().x;

    const float itemSpacing =
        ImGui::GetStyle().
        ItemSpacing.x;

    const float itemStride =
        previewWidth +
        itemSpacing;

    int itemsPerRow = 1;

    if (availableWidth > 0.0f &&
        itemStride > 0.0f)
    {
        itemsPerRow =
            std::max(
                1,
                static_cast<int>(
                    (
                        availableWidth +
                        itemSpacing
                        ) /
                    itemStride
                    )
            );
    }

    ImGui::BeginChild(
        "##TilePaletteScroll",
        ImVec2(
            0.0f,
            0.0f
        ),
        false
    );

    for (std::uint64_t index = 0;
        index < tileCount;
        ++index)
    {
        const std::uint64_t
            tileId64 =
            index + 1;

        if (tileId64 >
            static_cast<std::uint64_t>(
                std::numeric_limits<
                TileId
                >::max()
                ))
        {
            break;
        }

        const TileId tileId =
            static_cast<TileId>(
                tileId64
                );

        const UVRect uv =
            CalculateTileUV(
                tileId
            );

        ImGui::Image(
            reinterpret_cast<ImTextureID>(
                srv
                ),
            ImVec2(
                previewWidth,
                previewHeight
            ),
            ImVec2(
                uv.u0,
                uv.v0
            ),
            ImVec2(
                uv.u1,
                uv.v1
            )
        );

        const ImVec2 itemMin =
            ImGui::GetItemRectMin();

        const ImVec2 itemMax =
            ImGui::GetItemRectMax();

        if (ImGui::IsItemClicked(
            ImGuiMouseButton_Left
        ))
        {
            m_selectedTileId =
                tileId;
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();

            ImGui::Text(
                "TileId: %u",
                static_cast<unsigned int>(
                    tileId
                    )
            );

            ImGui::EndTooltip();
        }

        if (m_selectedTileId ==
            tileId)
        {
            ImDrawList* drawList =
                ImGui::GetWindowDrawList();

            if (drawList)
            {
                const ImU32 color =
                    ImGui::GetColorU32(
                        ImGuiCol_Text
                    );

                drawList->AddRect(
                    itemMin,
                    itemMax,
                    color,
                    0.0f,
                    0,
                    2.0f
                );
            }
        }

        const bool endOfRow =
            (
                (index + 1) %
                static_cast<std::uint64_t>(
                    itemsPerRow
                    )
                ) == 0;

        const bool lastTile =
            index + 1 ==
            tileCount;

        if (!endOfRow &&
            !lastTile)
        {
            ImGui::SameLine();
        }
    }

    ImGui::EndChild();
}

bool TilePalettePanel::
ValidateAgainstTexture(
    const TilesetData& data,
    const Texture& texture
) const noexcept
{
    if (!TilesetSerializer::Validate(
        data
    ))
    {
        return false;
    }

    if (texture.GetWidth() <= 0 ||
        texture.GetHeight() <= 0)
    {
        return false;
    }

    const std::int64_t tileWidth =
        static_cast<std::int64_t>(
            data.tileWidth
            );

    const std::int64_t tileHeight =
        static_cast<std::int64_t>(
            data.tileHeight
            );

    const std::int64_t columns =
        static_cast<std::int64_t>(
            data.columns
            );

    const std::int64_t rows =
        static_cast<std::int64_t>(
            data.rows
            );

    const std::int64_t margin =
        static_cast<std::int64_t>(
            data.margin
            );

    const std::int64_t spacing =
        static_cast<std::int64_t>(
            data.spacing
            );

    const std::int64_t strideX =
        tileWidth +
        spacing;

    const std::int64_t strideY =
        tileHeight +
        spacing;

    if (strideX <= 0 ||
        strideY <= 0)
    {
        return false;
    }

    const std::int64_t requiredWidth =
        margin +
        (
            columns - 1
            ) *
        strideX +
        tileWidth;

    const std::int64_t requiredHeight =
        margin +
        (
            rows - 1
            ) *
        strideY +
        tileHeight;

    if (requiredWidth <= 0 ||
        requiredHeight <= 0)
    {
        return false;
    }

    if (requiredWidth >
        static_cast<std::int64_t>(
            texture.GetWidth()
            ) ||
        requiredHeight >
        static_cast<std::int64_t>(
            texture.GetHeight()
            ))
    {
        return false;
    }

    const std::uint64_t tileCount =
        static_cast<std::uint64_t>(
            data.columns
            ) *
        static_cast<std::uint64_t>(
            data.rows
            );

    if (tileCount == 0 ||
        tileCount >
        static_cast<std::uint64_t>(
            std::numeric_limits<
            TileId
            >::max()
            ))
    {
        return false;
    }

    return true;
}

UVRect TilePalettePanel::
CalculateTileUV(
    TileId tileId
) const noexcept
{
    UVRect result
    {
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    if (!m_texture ||
        tileId ==
        InvalidTileId)
    {
        return result;
    }

    const std::uint64_t tileCount =
        GetTileCount();

    if (tileId >
        tileCount)
    {
        return result;
    }

    if (m_tilesetData.columns <= 0 ||
        m_tilesetData.rows <= 0)
    {
        return result;
    }

    const std::uint64_t zeroBased =
        static_cast<std::uint64_t>(
            tileId - 1
            );

    const std::uint64_t column =
        zeroBased %
        static_cast<std::uint64_t>(
            m_tilesetData.columns
            );

    const std::uint64_t row =
        zeroBased /
        static_cast<std::uint64_t>(
            m_tilesetData.columns
            );

    const std::int64_t strideX =
        static_cast<std::int64_t>(
            m_tilesetData.tileWidth
            ) +
        static_cast<std::int64_t>(
            m_tilesetData.spacing
            );

    const std::int64_t strideY =
        static_cast<std::int64_t>(
            m_tilesetData.tileHeight
            ) +
        static_cast<std::int64_t>(
            m_tilesetData.spacing
            );

    const std::int64_t pixelX =
        static_cast<std::int64_t>(
            m_tilesetData.margin
            ) +
        static_cast<std::int64_t>(
            column
            ) *
        strideX;

    const std::int64_t pixelY =
        static_cast<std::int64_t>(
            m_tilesetData.margin
            ) +
        static_cast<std::int64_t>(
            row
            ) *
        strideY;

    const std::int64_t pixelRight =
        pixelX +
        static_cast<std::int64_t>(
            m_tilesetData.tileWidth
            );

    const std::int64_t pixelBottom =
        pixelY +
        static_cast<std::int64_t>(
            m_tilesetData.tileHeight
            );

    const int textureWidth =
        m_texture->GetWidth();

    const int textureHeight =
        m_texture->GetHeight();

    if (pixelX < 0 ||
        pixelY < 0 ||
        pixelRight >
        textureWidth ||
        pixelBottom >
        textureHeight)
    {
        return result;
    }

    const float inverseWidth =
        1.0f /
        static_cast<float>(
            textureWidth
            );

    const float inverseHeight =
        1.0f /
        static_cast<float>(
            textureHeight
            );

    result.u0 =
        static_cast<float>(
            pixelX
            ) *
        inverseWidth;

    result.v0 =
        static_cast<float>(
            pixelY
            ) *
        inverseHeight;

    result.u1 =
        static_cast<float>(
            pixelRight
            ) *
        inverseWidth;

    result.v1 =
        static_cast<float>(
            pixelBottom
            ) *
        inverseHeight;

    return result;
}

std::uint64_t
TilePalettePanel::
GetTileCount() const noexcept
{
    if (m_tilesetData.columns <= 0 ||
        m_tilesetData.rows <= 0)
    {
        return 0;
    }

    return
        static_cast<std::uint64_t>(
            m_tilesetData.columns
            ) *
        static_cast<std::uint64_t>(
            m_tilesetData.rows
            );
}

std::string TilePalettePanel::ToUtf8(
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