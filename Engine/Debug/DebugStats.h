#pragma once

#include <cstdint>

struct DebugStats
{
    float fps = 0.0f;

    float frameTimeMs = 0.0f;

    int entityCount = 0;

    int drawCalls = 0;

    std::uint32_t fixedSteps = 0;

    float fixedUpdateHz = 60.0f;

    float interpolationAlpha = 0.0f;

    bool vsync = false;

    // 0 == Unlimited
    std::uint32_t targetFPS = 0;

    int renderCommands = 0;

    // --------------------------------------------------
    // TileMap Rendering
    // --------------------------------------------------

    std::uint32_t tileRenderItems = 0;

    std::uint32_t visibleTiles = 0;

    std::uint32_t culledTiles = 0;

    std::uint32_t tileRenderLayers = 0;

    int tileMapWidth = 0;
    int tileMapHeight = 0;

    int tileWidth = 0;
    int tileHeight = 0;

    // 현재 Camera가 검사하고 있는
    // Tile coordinate 범위.
    //
    // -1이면 Camera와 Map이 겹치지 않음.
    int visibleTileMinX = -1;
    int visibleTileMaxX = -1;

    int visibleTileMinY = -1;
    int visibleTileMaxY = -1;

    float cameraLeft = 0.0f;
    float cameraRight = 0.0f;

    float cameraTop = 0.0f;
    float cameraBottom = 0.0f;

    bool tileMapInView = false;

    std::uint32_t tileCandidateCells = 0;

    std::uint32_t tileGridCells = 0;

    std::uint32_t
        tileCollisionTiles = 0;

    std::uint32_t
        tileCollisionShapes = 0;

    std::uint32_t
        tileCollisionLayers = 0;

    std::uint32_t
        tileCollisionMergedArea = 0;

    void ResetTileStats()
    {
        tileRenderItems = 0;

        visibleTiles = 0;
        culledTiles = 0;

        tileRenderLayers = 0;

        tileMapWidth = 0;
        tileMapHeight = 0;

        tileWidth = 0;
        tileHeight = 0;

        visibleTileMinX = -1;
        visibleTileMaxX = -1;

        visibleTileMinY = -1;
        visibleTileMaxY = -1;

        cameraLeft = 0.0f;
        cameraRight = 0.0f;

        cameraTop = 0.0f;
        cameraBottom = 0.0f;

        tileMapInView = false;
    
        tileCandidateCells = 0;
        tileGridCells = 0;

        tileCollisionTiles = 0;
        tileCollisionShapes = 0;
        tileCollisionLayers = 0;

        tileCollisionMergedArea = 0;
    }
};