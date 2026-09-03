#include "TileMapRenderer.h"

#include "TileMap.h"
#include "Tileset.h"

#include "Engine/Graphics/Texture.h"

#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/RenderQueue.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <cassert>

bool TileMapRenderer::Build(
    const TileMap& tileMap)
{
    Clear();

    m_mapWidth =
        tileMap.GetWidth();

    m_mapHeight =
        tileMap.GetHeight();

    m_tileWidth =
        tileMap.GetTileWidth();

    m_tileHeight =
        tileMap.GetTileHeight();

    if (m_mapWidth <= 0 ||
        m_mapHeight <= 0)
    {
        Clear();

        return false;
    }

    if (m_tileWidth <= 0 ||
        m_tileHeight <= 0)
    {
        Clear();

        return false;
    }

    Tileset* tileset =
        tileMap.GetTileset();

    if (!tileset)
    {
        Clear();

        return false;
    }

    Texture* texture =
        tileset->GetTexture();

    if (!texture)
    {
        Clear();

        return false;
    }

    const std::size_t cellCount =
        static_cast<std::size_t>(
            m_mapWidth
            )
        *
        static_cast<std::size_t>(
            m_mapHeight
            );

    //
    // TileMap의 Render layer만
    // 별도 cache로 만든다.
    //

    for (
        std::size_t layerIndex = 0;
        layerIndex <
        tileMap.GetLayerCount();
        ++layerIndex)
    {
        const TileLayer* layer =
            tileMap.GetLayer(
                layerIndex
            );

        if (!layer)
        {
            continue;
        }

        if (layer->type !=
            TileLayerType::Render)
        {
            continue;
        }

        TileRenderLayerCache
            layerCache;

        layerCache.sourceLayerIndex =
            layerIndex;

        layerCache.visible =
            layer->visible;

        layerCache.cells.resize(
            cellCount
        );

        for (
            int y = 0;
            y < m_mapHeight;
            ++y)
        {
            for (
                int x = 0;
                x < m_mapWidth;
                ++x)
            {
                const TileId tileId =
                    tileMap.GetTile(
                        layerIndex,
                        x,
                        y
                    );

                if (tileId ==
                    InvalidTileId)
                {
                    continue;
                }

                if (!tileset->
                    IsValidTileId(
                        tileId
                    ))
                {
                    continue;
                }

                const std::size_t
                    cellIndex =
                    GetCellIndex(
                        x,
                        y
                    );

                TileRenderItem&
                    item =
                    layerCache.
                    cells[
                        cellIndex
                    ];

                item.occupied =
                    true;

                //
                // Sprite
                //

                item.sprite.texture =
                    texture;

                item.sprite.visible =
                    true;

                item.sprite.layer =
                    layer->renderLayer;

                item.sprite.zIndex =
                    static_cast<float>(
                        layerIndex
                        );

                item.sprite.useYSort =
                    false;

                item.sprite.blendMode =
                    BlendMode::Alpha;

                item.sprite.uv =
                    tileset->
                    GetTileUV(
                        tileId
                    );

                //
                // Transform
                //
                // 현재 Renderer contract:
                //
                // position = 좌상단
                // scale    = 실제 pixel size
                //

                item.transform.position =
                {
                    static_cast<float>(
                        x *
                        m_tileWidth
                    ),

                    static_cast<float>(
                        y *
                        m_tileHeight
                    )
                };

                item.transform.scale =
                {
                    static_cast<float>(
                        m_tileWidth
                    ),

                    static_cast<float>(
                        m_tileHeight
                    )
                };

                item.transform.rotation =
                    0.0f;

                ++layerCache.renderItemCount;
            }
        }

        if (layerCache.visible)
        {
            m_renderItemCount +=
                layerCache.renderItemCount;
        }


        m_renderLayers.emplace_back(
            std::move(
                layerCache
            )
        );

#ifdef _DEBUG

        for (const auto& renderLayer :
            m_renderLayers)
        {
            assert(
                renderLayer.cells.size() ==
                cellCount
            );
        }

#endif
    }

    m_stats.totalRenderItems =
        m_renderItemCount;

    m_stats.renderLayerCount =
        m_renderLayers.size();

    m_stats.mapWidth =
        m_mapWidth;

    m_stats.mapHeight =
        m_mapHeight;

    m_stats.tileWidth =
        m_tileWidth;

    m_stats.tileHeight =
        m_tileHeight;

    m_stats.totalGridCells =
        static_cast<std::size_t>(
            m_mapWidth
            )
        *
        static_cast<std::size_t>(
            m_mapHeight
            )
        *
        m_renderLayers.size();

    return true;
}

void TileMapRenderer::Clear()
{
    m_renderLayers.clear();

    m_mapWidth = 0;
    m_mapHeight = 0;

    m_tileWidth = 0;
    m_tileHeight = 0;

    m_renderItemCount = 0;

    m_stats =
        TileMapRenderStats{};
}

std::size_t
TileMapRenderer::GetCellIndex(
    int x,
    int y) const
{
#ifdef _DEBUG

    assert(
        x >= 0 &&
        x < m_mapWidth
    );

    assert(
        y >= 0 &&
        y < m_mapHeight
    );

#endif

    return
        static_cast<std::size_t>(
            y
            )
        *
        static_cast<std::size_t>(
            m_mapWidth
            )
        +
        static_cast<std::size_t>(
            x
            );
}

void TileMapRenderer::Submit(
    const Camera& camera,
    RenderQueue& renderQueue)
{
    //
    // --------------------------------------------
    // Static stats
    // --------------------------------------------
    //

    const std::size_t
        visibleRenderLayerCount =
        GetVisibleRenderLayerCount();

    m_stats.totalRenderItems =
        m_renderItemCount;

    m_stats.renderLayerCount =
        visibleRenderLayerCount;

    m_stats.mapWidth =
        m_mapWidth;

    m_stats.mapHeight =
        m_mapHeight;

    m_stats.tileWidth =
        m_tileWidth;

    m_stats.tileHeight =
        m_tileHeight;

    m_stats.totalGridCells =
        static_cast<std::size_t>(
            m_mapWidth)
        *
        static_cast<std::size_t>(
            m_mapHeight)
        *
        visibleRenderLayerCount;

    //
    // --------------------------------------------
    // Per-frame stats
    // --------------------------------------------
    //

    m_stats.visibleRenderItems = 0;

    m_stats.culledRenderItems =
        m_renderItemCount;

    m_stats.visibleCandidateCells =
        0;

    m_stats.visibleMinTileX = -1;
    m_stats.visibleMaxTileX = -1;

    m_stats.visibleMinTileY = -1;
    m_stats.visibleMaxTileY = -1;

    m_stats.mapInView = false;

    m_stats.cameraLeft =
        camera.GetLeft();

    m_stats.cameraRight =
        camera.GetRight();

    m_stats.cameraTop =
        camera.GetTop();

    m_stats.cameraBottom =
        camera.GetBottom();

    //
    // --------------------------------------------
    // Visible bounds
    // --------------------------------------------
    //

    const VisibleTileBounds bounds =
        CalculateVisibleTileBounds(
            camera
        );

    if (!bounds.valid)
    {
        return;
    }

    m_stats.mapInView = true;

    m_stats.visibleMinTileX =
        bounds.minX;

    m_stats.visibleMaxTileX =
        bounds.maxX;

    m_stats.visibleMinTileY =
        bounds.minY;

    m_stats.visibleMaxTileY =
        bounds.maxY;

    const std::size_t
        visibleCellCountPerLayer =
        static_cast<std::size_t>(
            bounds.GetWidth()
            )
        *
        static_cast<std::size_t>(
            bounds.GetHeight()
            );

    m_stats.visibleCandidateCells =
        visibleCellCountPerLayer
        *
        m_renderLayers.size();

    //
    // --------------------------------------------
    // Submit visible cells
    // --------------------------------------------
    //

    for (const auto& layer :
        m_renderLayers)
    {
        if (!layer.visible)
        {
            continue;
        }

        for (
            int y = bounds.minY;
            y <= bounds.maxY;
            ++y)
        {
            for (
                int x = bounds.minX;
                x <= bounds.maxX;
                ++x)
            {
                const std::size_t
                    cellIndex =
                    GetCellIndex(
                        x,
                        y
                    );

                const TileRenderItem&
                    item =
                    layer.cells[
                        cellIndex
                    ];

                if (!item.occupied)
                {
                    continue;
                }

                renderQueue.Submit(
                    item.sprite,
                    item.transform
                );

                ++m_stats.visibleRenderItems;
            }
        }
    }

    //
    // --------------------------------------------
    // Final stats
    // --------------------------------------------
    //

    if (m_renderItemCount >
        m_stats.visibleRenderItems)
    {
        m_stats.culledRenderItems =
            m_renderItemCount -
            m_stats.visibleRenderItems;
    }
    else
    {
        m_stats.culledRenderItems = 0;
    }
}

TileMapRenderer::VisibleTileBounds
TileMapRenderer::CalculateVisibleTileBounds(
    const Camera& camera) const
{
    VisibleTileBounds bounds;

    if (m_mapWidth <= 0 ||
        m_mapHeight <= 0)
    {
        return bounds;
    }

    if (m_tileWidth <= 0 ||
        m_tileHeight <= 0)
    {
        return bounds;
    }

    const float left =
        camera.GetLeft();

    const float right =
        camera.GetRight();

    const float top =
        camera.GetTop();

    const float bottom =
        camera.GetBottom();

    const float tileWidth =
        static_cast<float>(
            m_tileWidth
            );

    const float tileHeight =
        static_cast<float>(
            m_tileHeight
            );

    int minTileX =
        static_cast<int>(
            std::floor(
                left /
                tileWidth
            )
            );

    int minTileY =
        static_cast<int>(
            std::floor(
                top /
                tileHeight
            )
            );

    int maxTileX =
        static_cast<int>(
            std::ceil(
                right /
                tileWidth
            )
            ) - 1;

    int maxTileY =
        static_cast<int>(
            std::ceil(
                bottom /
                tileHeight
            )
            ) - 1;

    //
    // Camera와 TileMap이
    // 완전히 떨어져 있다.
    //
    // Clamp보다 먼저 검사해야 한다.
    //

    if (maxTileX < 0 ||
        maxTileY < 0 ||
        minTileX >= m_mapWidth ||
        minTileY >= m_mapHeight)
    {
        return bounds;
    }

    minTileX =
        std::max(
            minTileX,
            0
        );

    minTileY =
        std::max(
            minTileY,
            0
        );

    maxTileX =
        std::min(
            maxTileX,
            m_mapWidth - 1
        );

    maxTileY =
        std::min(
            maxTileY,
            m_mapHeight - 1
        );

    if (minTileX >
        maxTileX ||
        minTileY >
        maxTileY)
    {
        return bounds;
    }

    bounds.minX =
        minTileX;

    bounds.maxX =
        maxTileX;

    bounds.minY =
        minTileY;

    bounds.maxY =
        maxTileY;

    bounds.valid = true;

#ifdef _DEBUG

    assert(
        m_stats.visibleRenderItems <=
        m_stats.totalRenderItems
    );

    assert(
        m_stats.visibleRenderItems +
        m_stats.culledRenderItems ==
        m_stats.totalRenderItems
    );

    assert(
        m_stats.visibleRenderItems <=
        m_stats.visibleCandidateCells
    );

    assert(
        m_stats.visibleCandidateCells <=
        m_stats.totalGridCells
    );

#endif

    return bounds;
}

TileMapRenderer::TileRenderLayerCache*
TileMapRenderer::FindRenderLayer(
    std::size_t sourceLayerIndex)
{
    for (auto& layer :
        m_renderLayers)
    {
        if (layer.sourceLayerIndex ==
            sourceLayerIndex)
        {
            return &layer;
        }
    }

    return nullptr;
}

const TileMapRenderer::
TileRenderLayerCache*
TileMapRenderer::FindRenderLayer(
    std::size_t sourceLayerIndex) const
{
    for (const auto& layer :
        m_renderLayers)
    {
        if (layer.sourceLayerIndex ==
            sourceLayerIndex)
        {
            return &layer;
        }
    }

    return nullptr;
}

bool TileMapRenderer::
IsRenderLayerCached(
    std::size_t sourceLayerIndex) const
{
    return
        FindRenderLayer(
            sourceLayerIndex) != nullptr;
}

bool TileMapRenderer::
IsRenderLayerVisible(
    std::size_t sourceLayerIndex) const
{
    const TileRenderLayerCache* layer =
        FindRenderLayer(
            sourceLayerIndex);

    if (!layer)
    {
        return false;
    }

    return layer->visible;
}

bool TileMapRenderer::
SetRenderLayerVisible(
    std::size_t sourceLayerIndex,
    bool visible)
{
    TileRenderLayerCache* layer =
        FindRenderLayer(
            sourceLayerIndex);

    if (!layer)
    {
        return false;
    }

    if (layer->visible == visible)
    {
        return true;
    }

    if (visible)
    {
        m_renderItemCount +=
            layer->renderItemCount;
    }
    else
    {
#ifdef _DEBUG
        assert(
            m_renderItemCount >=
            layer->renderItemCount
        );
#endif

        m_renderItemCount -=
            layer->renderItemCount;
    }

    layer->visible = visible;

    return true;
}

std::size_t
TileMapRenderer::
GetVisibleRenderLayerCount() const
{
    std::size_t count = 0;

    for (const auto& layer :
        m_renderLayers)
    {
        if (layer.visible)
        {
            ++count;
        }
    }

    return count;
}