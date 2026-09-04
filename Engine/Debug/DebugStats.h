#pragma once

#include "Engine/Debug/CpuProfiler.h"
#include <cstdint>

struct DebugStats
{
    float fps = 0.0f;

    float frameTimeMs = 0.0f;

    // --------------------------------------------------
    // CPU Profiling
    // --------------------------------------------------

    float fixedUpdateCpuMs =
        0.0f;

    float fixedStepAverageCpuMs =
        0.0f;

    std::uint32_t profiledFixedSteps =
        0;

    float updateCpuMs =
        0.0f;

    float lateUpdateCpuMs =
        0.0f;

    float renderCpuMs =
        0.0f;

    float presentMs =
        0.0f;

    float engineCpuWorkMs =
        0.0f;

    // --------------------------------------------------
    // FixedUpdate subsystem profiling
    // --------------------------------------------------

    float sceneFixedCpuMs =
        0.0f;

    float physicsStepCpuMs =
        0.0f;

    float physicsSyncCpuMs =
        0.0f;

    float contactDispatchCpuMs =
        0.0f;

    float fixedOverheadCpuMs =
        0.0f;

    // --------------------------------------------------
    // Render subsystem profiling
    // --------------------------------------------------

    float renderSubmitCpuMs =
        0.0f;

    float renderSortCpuMs =
        0.0f;

    float renderExecuteCpuMs =
        0.0f;

    float debugRenderCpuMs =
        0.0f;

    float renderOverheadCpuMs =
        0.0f;

    // --------------------------------------------------
    // CPU Profiler History / Diagnostics
    // --------------------------------------------------

    std::uint32_t
        profilerHistoryFrames = 0;

    float
        cpuWorkAverageMs = 0.0f;

    float
        cpuWorkMaxMs = 0.0f;

    std::uint32_t
        cpuWorkMaxFramesAgo =
        InvalidCpuProfileFrameAge;


    float
        presentAverageMs = 0.0f;

    float
        presentMaxMs = 0.0f;

    float
        cpuSpikeThresholdMs = 0.0f;

    bool
        currentCpuSpike = false;

    std::uint32_t
        cpuSpikesInHistory = 0;

    std::uint32_t
        latestCpuSpikeFramesAgo =
        InvalidCpuProfileFrameAge;


    CpuProfileZone
        peakFrameWorstCpuPhase =
        CpuProfileZone::Count;

    float
        peakFrameWorstCpuPhaseMs =
        0.0f;


    CpuProfileZone
        peakFrameWorstSubsystem =
        CpuProfileZone::Count;

    float
        peakFrameWorstSubsystemMs =
        0.0f;

    int entityCount = 0;

    int drawCalls = 0;

    std::uint32_t fixedSteps = 0;

    float fixedUpdateHz = 60.0f;

    float interpolationAlpha = 0.0f;

    bool vsync = false;

    std::uint32_t targetFPS = 0;

    int renderCommands = 0;

    // --------------------------------------------------
    // Render Batching
    // --------------------------------------------------

    std::uint32_t renderBatches = 0;

    std::uint32_t
        batchedRenderCommands = 0;

    std::uint32_t
        maxBatchSize = 0;

    std::uint32_t
        singleCommandBatches = 0;

    std::uint32_t
        batchBoundaries = 0;

    std::uint32_t
        textureBatchBoundaries = 0;

    std::uint32_t
        blendBatchBoundaries = 0;

    std::uint32_t
        layerBatchBoundaries = 0;

    std::uint32_t
        invalidRenderCommands = 0;

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