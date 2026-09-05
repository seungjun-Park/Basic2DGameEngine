#include "TileMapEditorPanel.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Resource/ResourceManager.h"
#include "Engine/Serialization/TileMapSerializer.h"
#include "Engine/Serialization/TilesetSerializer.h"

#include <Windows.h>

#include <imgui.h>

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

    m_lastOpenFailed =
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

    m_lastOpenFailed =
        false;
}

void TileMapEditorPanel::DrawContents(
    ResourceManager& resourceManager
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

    const bool reloadRequested =
        ImGui::Button(
            "Reload"
        );

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

    ImGui::Separator();

    DrawDocumentInfo();

    ImGui::Separator();

    if (ImGui::BeginChild(
        "##TileMapLayerList",
        ImVec2(
            240.0f,
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
        "##TileMapLayerDetails",
        ImVec2(
            0.0f,
            0.0f
        ),
        true
    ))
    {
        DrawSelectedLayerInfo();
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
        index < m_data.layers.size();
        ++index)
    {
        const TileLayer& layer =
            m_data.layers[index];

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

    if (layer.type ==
        TileLayerType::Collision)
    {
        ImGui::TextDisabled(
            "Collision values: "
            "0 = Empty, 1 = Solid"
        );
    }
    else
    {
        ImGui::TextDisabled(
            "Render values: "
            "0 = Empty, 1..N = Tileset TileId"
        );
    }

    ImGui::Separator();

    ImGui::TextDisabled(
        "Tile painting is added in Stage 18."
    );
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

    const std::uint64_t tilesetTileCount =
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

                if (static_cast<std::uint64_t>(
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