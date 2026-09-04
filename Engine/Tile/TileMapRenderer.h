#pragma once

#include "Engine/Components/Sprite.h"
#include "Engine/Components/Transform.h"

#include <cstddef>
#include <vector>

class Camera;
class TileMap;
class RenderQueue;


struct TileMapRenderStats
{
    std::size_t totalRenderItems = 0;

    std::size_t visibleRenderItems = 0;

    std::size_t culledRenderItems = 0;

    std::size_t renderLayerCount = 0;

    int mapWidth = 0;
    int mapHeight = 0;

    int tileWidth = 0;
    int tileHeight = 0;

    int visibleMinTileX = -1;
    int visibleMaxTileX = -1;

    int visibleMinTileY = -1;
    int visibleMaxTileY = -1;

    float cameraLeft = 0.0f;
    float cameraRight = 0.0f;

    float cameraTop = 0.0f;
    float cameraBottom = 0.0f;

    bool mapInView = false;

    std::size_t visibleCandidateCells = 0;

    std::size_t totalGridCells = 0;
};


class TileMapRenderer
{
public:
    TileMapRenderer() = default;
    TileMapRenderer(
        const TileMapRenderer&
    ) = delete;

    ~TileMapRenderer() = default;

    TileMapRenderer&
        operator=(
            const TileMapRenderer&
            ) = delete;

    bool Build(
        const TileMap& tileMap
    );

    void Clear();

    void Submit(
        const Camera& camera,
        RenderQueue& renderQueue
    );

    bool SetRenderLayerVisible(
        std::size_t sourceLayerIndex,
        bool visible
    );

    std::size_t
        GetRenderItemCount() const
    {
        return
            m_renderItemCount;
    }

    const TileMapRenderStats&
        GetStats() const
    {
        return m_stats;
    }

    [[nodiscard]]
    bool IsRenderLayerVisible(
        std::size_t sourceLayerIndex
    ) const;

    [[nodiscard]]
    bool IsRenderLayerCached(
        std::size_t sourceLayerIndex
    ) const;

private:
    struct TileRenderItem
    {
        Sprite sprite;
        Transform transform;

        bool occupied = false;
    };

    struct TileRenderLayerCache
    {
        std::size_t sourceLayerIndex = 0;

        bool visible = true;

        std::size_t renderItemCount = 0;

        std::vector<TileRenderItem>
            cells;
    };

    struct VisibleTileBounds
    {
        int minX = -1;
        int maxX = -1;

        int minY = -1;
        int maxY = -1;

        bool valid = false;

        int GetWidth() const
        {
            if (!valid)
            {
                return 0;
            }

            return
                maxX -
                minX +
                1;
        }

        int GetHeight() const
        {
            if (!valid)
            {
                return 0;
            }

            return
                maxY -
                minY +
                1;
        }
    };

private:
    VisibleTileBounds
        CalculateVisibleTileBounds(
            const Camera& camera
        ) const;

    TileRenderLayerCache*
        FindRenderLayer(
            std::size_t sourceLayerIndex
        );

    const TileRenderLayerCache*
        FindRenderLayer(
            std::size_t sourceLayerIndex
        ) const;

    std::size_t
        GetVisibleRenderLayerCount() const;

    std::size_t GetCellIndex(
        int x,
        int y
    ) const;

private:
    int m_mapWidth = 0;
    int m_mapHeight = 0;

    int m_tileWidth = 0;
    int m_tileHeight = 0;

    std::vector<TileRenderLayerCache>
        m_renderLayers;

    std::size_t
        m_renderItemCount = 0;

    TileMapRenderStats m_stats;
};