#include "TileMapEditorPanel.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TileMapSerializer.h"
#include "Engine/Serialization/TilesetSerializer.h"
#include "Engine/Tile/ITileMapRuntimeTarget.h"

#include <Windows.h>

#include <imgui.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

namespace
{
    bool CalculateRequiredExtent(
        int tileSize,
        int count,
        int margin,
        int spacing,
        std::int64_t& result
    ) noexcept
    {
        result = 0;

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

        const std::int64_t marginValue =
            static_cast<std::int64_t>(
                margin
                );

        const std::int64_t spacingValue =
            static_cast<std::int64_t>(
                spacing
                );

        const std::int64_t stride =
            tile +
            spacingValue;

        const std::int64_t steps =
            itemCount -
            1;

        constexpr std::int64_t maximum =
            std::numeric_limits<
            std::int64_t
            >::max();

        if (marginValue >
            maximum - tile)
        {
            return false;
        }

        const std::int64_t base =
            marginValue +
            tile;

        if (steps > 0)
        {
            if (stride <= 0)
            {
                return false;
            }

            if (steps >
                (maximum - base) /
                stride)
            {
                return false;
            }
        }

        result =
            base +
            steps *
            stride;

        return true;
    }
}

bool TileMapEditorPanel::Open(
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

    auto mapData =
        TileMapSerializer::Load(
            path
        );

    if (!mapData)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    auto tilesetData =
        TilesetSerializer::Load(
            mapData->tilesetPath
        );

    if (!tilesetData)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    Texture* texture =
        resourceManager.LoadTexture(
            tilesetData->texturePath
        );

    if (!texture)
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    if (!ValidateAgainstTileset(
        *mapData,
        *tilesetData,
        *texture
    ))
    {
        m_lastOpenFailed =
            true;

        return false;
    }

    m_documentPath =
        path;

    m_data =
        std::move(
            *mapData
        );

    m_tilesetData =
        std::move(
            *tilesetData
        );

    m_texture =
        texture;

    if (m_data.layers.empty())
    {
        m_selectedLayerIndex =
            InvalidLayerIndex;
    }
    else
    {
        m_selectedLayerIndex =
            0;
    }

    m_collisionBrush =
        SolidCollisionTile;

    m_lastOpenFailed =
        false;

    m_lastRuntimeApplySucceeded =
        false;

    m_lastRuntimeApplyFailed =
        false;

    return true;
}

void TileMapEditorPanel::Close()
noexcept
{
    m_documentPath.clear();

    m_data =
        TileMapData{};

    m_tilesetData =
        TilesetData{};

    m_texture =
        nullptr;

    m_selectedLayerIndex =
        InvalidLayerIndex;

    m_collisionBrush =
        SolidCollisionTile;

    m_canvasCellSize =
        48.0f;

    m_showGrid =
        true;

    m_lastOpenFailed =
        false;

    m_lastRuntimeApplySucceeded =
        false;

    m_lastRuntimeApplyFailed =
        false;
}

void TileMapEditorPanel::DrawContents(
    ResourceManager& resourceManager,
    TileId paletteTileId,
    const std::wstring& paletteTilesetPath,
    bool allowEditing,
    ITileMapRuntimeTarget* runtimeTarget
)
{
    if (!IsOpen())
    {
        ImGui::TextDisabled(
            "No TileMap document is open."
        );

        if (m_lastOpenFailed)
        {
            ImGui::TextDisabled(
                "The last TileMap open "
                "request failed."
            );
        }

        return;
    }

    const bool runtimeUsesDocument =
        runtimeTarget &&
        runtimeTarget->IsUsingTileMap(
            m_documentPath
        );

    const bool runtimeApplyAvailable =
        allowEditing &&
        runtimeUsesDocument &&
        TileMapSerializer::Validate(
            m_data
        );

    //
    // Toolbar
    //

    const bool reloadRequested =
        ImGui::Button(
            "Reload"
        );

    ImGui::SameLine();

    ImGui::BeginDisabled(
        !runtimeApplyAvailable
    );

    const bool applyRuntimeRequested =
        ImGui::Button(
            "Apply Runtime"
        );

    ImGui::EndDisabled();

    ImGui::SameLine();

    const bool closeRequested =
        ImGui::Button(
            "Close"
        );

    if (reloadRequested)
    {
        Reload(
            resourceManager
        );
    }

    if (applyRuntimeRequested &&
        runtimeTarget)
    {
        m_lastRuntimeApplySucceeded =
            false;

        m_lastRuntimeApplyFailed =
            false;

        if (runtimeTarget->
            ApplyTileMapData(
                m_documentPath,
                m_data
            ))
        {
            m_lastRuntimeApplySucceeded =
                true;
        }
        else
        {
            m_lastRuntimeApplyFailed =
                true;
        }
    }

    if (closeRequested)
    {
        Close();

        return;
    }

    if (m_lastOpenFailed)
    {
        ImGui::TextDisabled(
            "Reload failed. Current document "
            "was preserved."
        );
    }

    if (!allowEditing)
    {
        ImGui::TextDisabled(
            "Tile painting and runtime apply "
            "are disabled in Play Mode."
        );
    }

    if (!runtimeTarget)
    {
        ImGui::TextDisabled(
            "The active Scene does not support "
            "runtime TileMap rebuild."
        );
    }
    else if (!runtimeUsesDocument)
    {
        ImGui::TextDisabled(
            "The active Scene is not using "
            "this TileMap document."
        );
    }

    if (m_lastRuntimeApplySucceeded)
    {
        ImGui::TextDisabled(
            "Runtime TileMap rebuild succeeded."
        );
    }

    if (m_lastRuntimeApplyFailed)
    {
        ImGui::TextDisabled(
            "Runtime TileMap rebuild failed. "
            "The previous runtime state was preserved."
        );
    }

    ImGui::TextDisabled(
        "Apply Runtime does not save JSON."
    );

    ImGui::Separator();

    DrawDocumentInfo();

    ImGui::Separator();

    if (ImGui::BeginChild(
        "##TileMapLayerList",
        ImVec2(
            220.0f,
            0.0f
        ),
        true
    ))
    {
        DrawLayers();
    }

    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginChild(
        "##TileMapEditorWorkspace",
        ImVec2(
            0.0f,
            0.0f
        ),
        true
    ))
    {
        DrawSelectedLayerInfo();

        if (HasSelectedLayer())
        {
            ImGui::Separator();

            DrawCanvas(
                paletteTileId,
                paletteTilesetPath,
                allowEditing
            );
        }
    }

    ImGui::EndChild();
}

bool TileMapEditorPanel::IsOpen()
const noexcept
{
    return
        !m_documentPath.empty() &&
        m_texture != nullptr;
}

const std::wstring&
TileMapEditorPanel::
GetDocumentPath() const noexcept
{
    return
        m_documentPath;
}

const std::wstring&
TileMapEditorPanel::
GetTilesetPath() const noexcept
{
    return
        m_data.tilesetPath;
}

std::size_t
TileMapEditorPanel::
GetSelectedLayerIndex() const noexcept
{
    return
        m_selectedLayerIndex;
}

bool TileMapEditorPanel::
HasSelectedLayer() const noexcept
{
    return
        m_selectedLayerIndex !=
        InvalidLayerIndex &&
        m_selectedLayerIndex <
        m_data.layers.size();
}

bool TileMapEditorPanel::Reload(
    ResourceManager& resourceManager
)
{
    if (m_documentPath.empty())
    {
        return false;
    }

    const std::wstring path =
        m_documentPath;

    return
        Open(
            path,
            resourceManager
        );
}

void TileMapEditorPanel::
DrawDocumentInfo()
{
    const std::string documentPath =
        ToUtf8(
            m_documentPath
        );

    const std::string tilesetPath =
        ToUtf8(
            m_data.tilesetPath
        );

    const std::string texturePath =
        ToUtf8(
            m_tilesetData.texturePath
        );

    ImGui::TextUnformatted(
        "TileMap Document"
    );

    ImGui::TextWrapped(
        "%s",
        documentPath.empty()
        ? "<invalid path>"
        : documentPath.c_str()
    );

    ImGui::Text(
        "Map Size: %d x %d",
        m_data.width,
        m_data.height
    );

    ImGui::Text(
        "Tile Size: %d x %d",
        m_data.tileWidth,
        m_data.tileHeight
    );

    ImGui::Text(
        "Layer Count: %zu",
        m_data.layers.size()
    );

    ImGui::TextWrapped(
        "Tileset: %s",
        tilesetPath.empty()
        ? "<invalid path>"
        : tilesetPath.c_str()
    );

    ImGui::TextWrapped(
        "Texture: %s",
        texturePath.empty()
        ? "<invalid path>"
        : texturePath.c_str()
    );
}

void TileMapEditorPanel::DrawLayers()
{
    ImGui::TextUnformatted(
        "Layers"
    );

    ImGui::Separator();

    if (m_data.layers.empty())
    {
        ImGui::TextDisabled(
            "No layers."
        );

        return;
    }

    for (std::size_t index = 0;
        index <
        m_data.layers.size();
        ++index)
    {
        const TileLayer& layer =
            m_data.layers[
                index
            ];

        const bool selected =
            m_selectedLayerIndex ==
            index;

        ImGui::PushID(
            static_cast<int>(
                index
                )
        );

        const char* label =
            layer.name.empty()
            ? "<Unnamed Layer>"
            : layer.name.c_str();

        const bool clicked =
            ImGui::Selectable(
                label,
                selected
            );

        ImGui::PopID();

        if (clicked)
        {
            m_selectedLayerIndex =
                index;
        }
    }
}

void TileMapEditorPanel::
DrawSelectedLayerInfo()
{
    if (!HasSelectedLayer())
    {
        ImGui::TextDisabled(
            "No layer selected."
        );

        return;
    }

    const TileLayer& layer =
        m_data.layers[
            m_selectedLayerIndex
        ];

    ImGui::TextUnformatted(
        "Layer Details"
    );

    ImGui::Separator();

    ImGui::Text(
        "Index: %zu",
        m_selectedLayerIndex
    );

    ImGui::Text(
        "Name: %s",
        layer.name.empty()
        ? "<Unnamed>"
        : layer.name.c_str()
    );

    ImGui::Text(
        "Type: %s",
        GetLayerTypeName(
            layer.type
        )
    );

    if (layer.type ==
        TileLayerType::Render)
    {
        ImGui::Text(
            "Render Layer: %s",
            GetRenderLayerName(
                layer.renderLayer
            )
        );
    }

    ImGui::Text(
        "Serialized Visibility: %s",
        layer.visible
        ? "Visible"
        : "Hidden"
    );

    ImGui::Text(
        "Tile Count: %zu",
        layer.tiles.size()
    );
}

void TileMapEditorPanel::DrawCanvas(
    TileId paletteTileId,
    const std::wstring& paletteTilesetPath,
    bool allowEditing
)
{
    if (!HasSelectedLayer() ||
        !m_texture)
    {
        return;
    }

    TileLayer& layer =
        m_data.layers[
            m_selectedLayerIndex
        ];

    ImGui::TextUnformatted(
        "Paint Canvas"
    );

    ImGui::SliderFloat(
        "Cell Size",
        &m_canvasCellSize,
        16.0f,
        128.0f,
        "%.0f px"
    );

    m_canvasCellSize =
        std::clamp(
            m_canvasCellSize,
            16.0f,
            128.0f
        );

    ImGui::Checkbox(
        "Show Grid",
        &m_showGrid
    );

    if (layer.type ==
        TileLayerType::Render)
    {
        const bool paletteMatches =
            !paletteTilesetPath.empty() &&
            paletteTilesetPath ==
            m_data.tilesetPath;

        if (paletteTileId ==
            InvalidTileId)
        {
            ImGui::Text(
                "Brush: Eraser / Empty [0]"
            );
        }
        else
        {
            ImGui::Text(
                "Brush TileId: %u",
                static_cast<unsigned int>(
                    paletteTileId
                    )
            );
        }

        if (!paletteMatches &&
            paletteTileId !=
            InvalidTileId)
        {
            ImGui::TextDisabled(
                "The Tile Palette uses a "
                "different Tileset."
            );
        }

        ImGui::TextDisabled(
            "Left drag = Paint, "
            "Right drag = Erase"
        );
    }
    else
    {
        ImGui::TextUnformatted(
            "Collision Brush"
        );

        const bool emptySelected =
            m_collisionBrush ==
            EmptyCollisionTile;

        if (ImGui::Selectable(
            "Empty [0]",
            emptySelected,
            0,
            ImVec2(
                100.0f,
                0.0f
            )
        ))
        {
            m_collisionBrush =
                EmptyCollisionTile;
        }

        ImGui::SameLine();

        const bool solidSelected =
            m_collisionBrush ==
            SolidCollisionTile;

        if (ImGui::Selectable(
            "Solid [1]",
            solidSelected,
            0,
            ImVec2(
                100.0f,
                0.0f
            )
        ))
        {
            m_collisionBrush =
                SolidCollisionTile;
        }

        ImGui::TextDisabled(
            "Left drag = Apply collision brush, "
            "Right drag = Empty"
        );
    }

    ImGui::Separator();

    if (m_data.width <= 0 ||
        m_data.height <= 0)
    {
        ImGui::TextDisabled(
            "Invalid map dimensions."
        );

        return;
    }

    const double canvasWidthDouble =
        static_cast<double>(
            m_data.width
            ) *
        static_cast<double>(
            m_canvasCellSize
            );

    const double canvasHeightDouble =
        static_cast<double>(
            m_data.height
            ) *
        static_cast<double>(
            m_canvasCellSize
            );

    if (canvasWidthDouble <= 0.0 ||
        canvasHeightDouble <= 0.0 ||
        canvasWidthDouble >
        static_cast<double>(
            std::numeric_limits<
            float
            >::max()
            ) ||
        canvasHeightDouble >
        static_cast<double>(
            std::numeric_limits<
            float
            >::max()
            ))
    {
        ImGui::TextDisabled(
            "Canvas dimensions are invalid."
        );

        return;
    }

    const ImVec2 canvasSize
    {
        static_cast<float>(
            canvasWidthDouble
        ),

        static_cast<float>(
            canvasHeightDouble
        )
    };

    constexpr ImGuiWindowFlags
        canvasFlags =
        ImGuiWindowFlags_HorizontalScrollbar;

    ImGui::BeginChild(
        "##TileMapPaintCanvasScroll",
        ImVec2(
            0.0f,
            0.0f
        ),
        true,
        canvasFlags
    );

    ImGui::InvisibleButton(
        "##TileMapPaintSurface",
        canvasSize
    );

    const ImVec2 canvasMin =
        ImGui::GetItemRectMin();

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    ID3D11ShaderResourceView* srv =
        m_texture->GetSRV();

    const ImU32 gridColor =
        ImGui::GetColorU32(
            ImGuiCol_Separator
        );

    const ImU32 collisionColor =
        ImGui::GetColorU32(
            ImGuiCol_ButtonActive
        );

    const ImU32 hoverColor =
        ImGui::GetColorU32(
            ImGuiCol_HeaderHovered
        );

    if (drawList)
    {
        for (int y = 0;
            y < m_data.height;
            ++y)
        {
            for (int x = 0;
                x < m_data.width;
                ++x)
            {
                const std::size_t tileIndex =
                    static_cast<std::size_t>(
                        y
                        ) *
                    static_cast<std::size_t>(
                        m_data.width
                        ) +
                    static_cast<std::size_t>(
                        x
                        );

                if (tileIndex >=
                    layer.tiles.size())
                {
                    continue;
                }

                const float left =
                    canvasMin.x +
                    static_cast<float>(
                        x
                        ) *
                    m_canvasCellSize;

                const float top =
                    canvasMin.y +
                    static_cast<float>(
                        y
                        ) *
                    m_canvasCellSize;

                const ImVec2 cellMin
                {
                    left,
                    top
                };

                const ImVec2 cellMax
                {
                    left +
                        m_canvasCellSize,

                    top +
                        m_canvasCellSize
                };

                const TileId tileId =
                    layer.tiles[
                        tileIndex
                    ];

                if (layer.type ==
                    TileLayerType::Render)
                {
                    if (srv &&
                        tileId !=
                        InvalidTileId)
                    {
                        const UVRect uv =
                            CalculateTileUV(
                                tileId
                            );

                        drawList->AddImage(
                            reinterpret_cast<
                            ImTextureID
                            >(
                                srv
                                ),
                            cellMin,
                            cellMax,
                            ImVec2(
                                uv.u0,
                                uv.v0
                            ),
                            ImVec2(
                                uv.u1,
                                uv.v1
                            )
                        );
                    }
                }
                else if (
                    tileId ==
                    SolidCollisionTile)
                {
                    drawList->
                        AddRectFilled(
                            cellMin,
                            cellMax,
                            collisionColor
                        );
                }

                if (m_showGrid)
                {
                    drawList->AddRect(
                        cellMin,
                        cellMax,
                        gridColor
                    );
                }
            }
        }
    }

    const bool canvasHovered =
        ImGui::IsItemHovered();

    if (canvasHovered)
    {
        const ImVec2 mousePosition =
            ImGui::GetMousePos();

        const float localX =
            mousePosition.x -
            canvasMin.x;

        const float localY =
            mousePosition.y -
            canvasMin.y;

        if (localX >= 0.0f &&
            localY >= 0.0f)
        {
            const int cellX =
                static_cast<int>(
                    localX /
                    m_canvasCellSize
                    );

            const int cellY =
                static_cast<int>(
                    localY /
                    m_canvasCellSize
                    );

            const bool inside =
                cellX >= 0 &&
                cellY >= 0 &&
                cellX <
                m_data.width &&
                cellY <
                m_data.height;

            if (inside)
            {
                const ImVec2 hoverMin
                {
                    canvasMin.x +
                        static_cast<float>(
                            cellX
                        ) *
                        m_canvasCellSize,

                    canvasMin.y +
                        static_cast<float>(
                            cellY
                        ) *
                        m_canvasCellSize
                };

                const ImVec2 hoverMax
                {
                    hoverMin.x +
                        m_canvasCellSize,

                    hoverMin.y +
                        m_canvasCellSize
                };

                if (drawList)
                {
                    drawList->AddRect(
                        hoverMin,
                        hoverMax,
                        hoverColor,
                        0.0f,
                        0,
                        2.0f
                    );
                }

                const std::size_t tileIndex =
                    static_cast<std::size_t>(
                        cellY
                        ) *
                    static_cast<std::size_t>(
                        m_data.width
                        ) +
                    static_cast<std::size_t>(
                        cellX
                        );

                if (tileIndex <
                    layer.tiles.size())
                {
                    const bool leftDown =
                        ImGui::IsMouseDown(
                            ImGuiMouseButton_Left
                        );

                    const bool rightDown =
                        ImGui::IsMouseDown(
                            ImGuiMouseButton_Right
                        );

                    if (!leftDown &&
                        !rightDown)
                    {
                        ImGui::BeginTooltip();

                        ImGui::Text(
                            "Cell: (%d, %d)",
                            cellX,
                            cellY
                        );

                        ImGui::Text(
                            "Value: %u",
                            static_cast<
                            unsigned int
                            >(
                                layer.tiles[
                                    tileIndex
                                ]
                                )
                        );

                        ImGui::EndTooltip();
                    }

                    if (allowEditing)
                    {
                        if (rightDown)
                        {
                            PaintCell(
                                cellX,
                                cellY,
                                paletteTileId,
                                paletteTilesetPath,
                                true
                            );
                        }
                        else if (leftDown)
                        {
                            PaintCell(
                                cellX,
                                cellY,
                                paletteTileId,
                                paletteTilesetPath,
                                false
                            );
                        }
                    }
                }
            }
        }
    }

    ImGui::EndChild();
}

void TileMapEditorPanel::PaintCell(
    int x,
    int y,
    TileId paletteTileId,
    const std::wstring& paletteTilesetPath,
    bool erase
)
{
    if (!HasSelectedLayer())
    {
        return;
    }

    if (x < 0 ||
        y < 0 ||
        x >= m_data.width ||
        y >= m_data.height)
    {
        return;
    }

    TileLayer& layer =
        m_data.layers[
            m_selectedLayerIndex
        ];

    const std::size_t index =
        static_cast<std::size_t>(
            y
            ) *
        static_cast<std::size_t>(
            m_data.width
            ) +
        static_cast<std::size_t>(
            x
            );

    if (index >=
        layer.tiles.size())
    {
        return;
    }

    TileId newValue =
        layer.tiles[
            index
        ];

    if (layer.type ==
        TileLayerType::Collision)
    {
        newValue =
            erase
            ? EmptyCollisionTile
            : m_collisionBrush;
    }
    else
    {
        if (erase ||
            paletteTileId ==
            InvalidTileId)
        {
            newValue =
                InvalidTileId;
        }
        else
        {
            if (!CanPaintRenderTile(
                paletteTileId,
                paletteTilesetPath
            ))
            {
                return;
            }

            newValue =
                paletteTileId;
        }
    }

    if (layer.tiles[index] ==
        newValue)
    {
        return;
    }

    layer.tiles[index] =
        newValue;

    //
    // Runtime is now stale relative to draft.
    //

    m_lastRuntimeApplySucceeded =
        false;

    m_lastRuntimeApplyFailed =
        false;
}

bool TileMapEditorPanel::
ValidateAgainstTileset(
    const TileMapData& mapData,
    const TilesetData& tilesetData,
    const Texture& texture
) const noexcept
{
    if (!TileMapSerializer::Validate(
        mapData
    ) ||
        !TilesetSerializer::Validate(
            tilesetData
        ))
    {
        return false;
    }

    if (texture.GetWidth() <= 0 ||
        texture.GetHeight() <= 0)
    {
        return false;
    }

    std::int64_t requiredWidth = 0;
    std::int64_t requiredHeight = 0;

    if (!CalculateRequiredExtent(
        tilesetData.tileWidth,
        tilesetData.columns,
        tilesetData.margin,
        tilesetData.spacing,
        requiredWidth
    ) ||
        !CalculateRequiredExtent(
            tilesetData.tileHeight,
            tilesetData.rows,
            tilesetData.margin,
            tilesetData.spacing,
            requiredHeight
        ))
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

    const std::uint64_t
        tilesetTileCount =
        static_cast<std::uint64_t>(
            tilesetData.columns
            ) *
        static_cast<std::uint64_t>(
            tilesetData.rows
            );

    if (tilesetTileCount == 0)
    {
        return false;
    }

    for (const TileLayer& layer :
        mapData.layers)
    {
        if (layer.type ==
            TileLayerType::Render)
        {
            for (TileId tileId :
            layer.tiles)
            {
                if (tileId ==
                    InvalidTileId)
                {
                    continue;
                }

                if (static_cast<
                    std::uint64_t
                >(
                    tileId
                    ) >
                    tilesetTileCount)
                {
                    return false;
                }
            }
        }
        else
        {
            for (TileId tileId :
            layer.tiles)
            {
                if (tileId !=
                    EmptyCollisionTile &&
                    !IsSolidCollisionTile(
                        tileId
                    ))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool TileMapEditorPanel::
CanPaintRenderTile(
    TileId tileId,
    const std::wstring& paletteTilesetPath
) const noexcept
{
    if (tileId ==
        InvalidTileId)
    {
        return true;
    }

    if (paletteTilesetPath.empty() ||
        paletteTilesetPath !=
        m_data.tilesetPath)
    {
        return false;
    }

    const std::uint64_t tileCount =
        GetTilesetTileCount();

    if (tileCount == 0)
    {
        return false;
    }

    return
        static_cast<std::uint64_t>(
            tileId
            ) <=
        tileCount;
}

UVRect TileMapEditorPanel::
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
        GetTilesetTileCount();

    if (tileCount == 0 ||
        static_cast<std::uint64_t>(
            tileId
            ) >
        tileCount)
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

    if (textureWidth <= 0 ||
        textureHeight <= 0)
    {
        return result;
    }

    if (pixelX < 0 ||
        pixelY < 0 ||
        pixelRight >
        static_cast<std::int64_t>(
            textureWidth
            ) ||
        pixelBottom >
        static_cast<std::int64_t>(
            textureHeight
            ))
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
TileMapEditorPanel::
GetTilesetTileCount() const noexcept
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

const char*
TileMapEditorPanel::
GetLayerTypeName(
    TileLayerType type
) noexcept
{
    switch (type)
    {
    case TileLayerType::Render:
        return "Render";

    case TileLayerType::Collision:
        return "Collision";

    default:
        return "Unknown";
    }
}

const char*
TileMapEditorPanel::
GetRenderLayerName(
    RenderLayer layer
) noexcept
{
    switch (layer)
    {
    case RenderLayer::Background:
        return "Background";

    case RenderLayer::World:
        return "World";

    case RenderLayer::Effect:
        return "Effect";

    case RenderLayer::Foreground:
        return "Foreground";

    case RenderLayer::UI:
        return "UI";

    case RenderLayer::Debug:
        return "Debug";

    default:
        return "Unknown";
    }
}

std::string
TileMapEditorPanel::ToUtf8(
    const std::wstring& value
)
{
    if (value.empty())
    {
        return {};
    }

    if (value.size() >
        static_cast<std::size_t>(
            std::numeric_limits<
            int
            >::max()
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