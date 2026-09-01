#include "TileMapRenderer.h"

#include "TileMap.h"
#include "Tileset.h"

#include "Engine/Graphics/Texture.h"

#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/RenderQueue.h"

#include <algorithm>
#include <cmath>
#include <utility>

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

        if (!layer->visible)
        {
            continue;
        }

        TileRenderLayerCache
            layerCache;

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

                ++m_renderItemCount;
            }
        }

        m_renderLayers.emplace_back(
            std::move(
                layerCache
            )
        );
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
    // Static stats
    //

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

    //
    // Per-frame stats reset
    //

    m_stats.visibleRenderItems = 0;

    m_stats.culledRenderItems =
        m_renderItemCount;

    m_stats.visibleMinTileX = -1;
    m_stats.visibleMaxTileX = -1;

    m_stats.visibleMinTileY = -1;
    m_stats.visibleMaxTileY = -1;

    m_stats.mapInView = false;

    //
    // Camera bounds
    //

    m_stats.cameraLeft =
        camera.GetLeft();

    m_stats.cameraRight =
        camera.GetRight();

    m_stats.cameraTop =
        camera.GetTop();

    m_stats.cameraBottom =
        camera.GetBottom();

    if (m_mapWidth <= 0 ||
        m_mapHeight <= 0)
    {
        return;
    }

    if (m_tileWidth <= 0 ||
        m_tileHeight <= 0)
    {
        return;
    }

    if (m_renderLayers.empty())
    {
        return;
    }

    const float left =
        m_stats.cameraLeft;

    const float right =
        m_stats.cameraRight;

    const float top =
        m_stats.cameraTop;

    const float bottom =
        m_stats.cameraBottom;

    //
    // 1. Camera visible world bounds
    //

    /*const float left =
        camera.GetLeft();

    const float right =
        camera.GetRight();

    const float top =
        camera.GetTop();

    const float bottom =
        camera.GetBottom();*/

    //
    // 2. World coordinate
    //    → Tile coordinate
    //
    // visible range는
    //
    // [left, right)
    // [top, bottom)
    //
    // 으로 취급한다.
    //as

    int minTileX =
        static_cast<int>(
            std::floor(
                left /
                static_cast<float>(
                    m_tileWidth
                    )
            )
            );

    int minTileY =
        static_cast<int>(
            std::floor(
                top /
                static_cast<float>(
                    m_tileHeight
                    )
            )
            );

    int maxTileX =
        static_cast<int>(
            std::ceil(
                right /
                static_cast<float>(
                    m_tileWidth
                    )
            )
            ) - 1;

    int maxTileY =
        static_cast<int>(
            std::ceil(
                bottom /
                static_cast<float>(
                    m_tileHeight
                    )
            )
            ) - 1;

    //
    // 3. Camera가 Map과 완전히 떨어져 있으면
    //    바로 종료.
    //
    // clamp 전에 검사해야 한다.
    //

    if (maxTileX < 0 ||
        maxTileY < 0 ||
        minTileX >= m_mapWidth ||
        minTileY >= m_mapHeight)
    {
        return;
    }

    //
    // 4. Map bounds로 clamp
    //

    minTileX =
        max(
            minTileX,
            0
        );

    minTileY =
        max(
            minTileY,
            0
        );

    maxTileX =
        min(
            maxTileX,
            m_mapWidth - 1
        );

    maxTileY =
        min(
            maxTileY,
            m_mapHeight - 1
        );

    if (minTileX >
        maxTileX ||
        minTileY >
        maxTileY)
    {
        return;
    }

    m_stats.mapInView = true;

    m_stats.visibleMinTileX =
        minTileX;

    m_stats.visibleMaxTileX =
        maxTileX;

    m_stats.visibleMinTileY =
        minTileY;

    m_stats.visibleMaxTileY =
        maxTileY;

    //
    // 5. Visible grid cell만 Submit
    //

    for (const auto& layer :
        m_renderLayers)
    {
        for (
            int y = minTileY;
            y <= maxTileY;
            ++y)
        {
            for (
                int x = minTileX;
                x <= maxTileX;
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
    if (m_renderItemCount >
        m_stats.visibleRenderItems)
    {
        m_stats.culledRenderItems =
            m_renderItemCount -
            m_stats.visibleRenderItems;
    }
    else
    {
        m_stats.culledRenderItems =
            0;
    }
}