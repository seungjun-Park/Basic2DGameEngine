#pragma once

#include "RenderCommand.h"

#include <cstddef>
#include <cstdint>
#include <vector>

struct Sprite;
struct Transform;

class SpriteRenderer;

struct RenderBatchStats
{
    std::size_t batchCount = 0;

    std::size_t
        batchedCommandCount = 0;

    std::size_t
        maxBatchSize = 0;

    std::size_t
        singleCommandBatchCount = 0;

    std::size_t
        batchBoundaryCount = 0;

    std::size_t
        textureBoundaryCount = 0;

    std::size_t
        blendBoundaryCount = 0;

    std::size_t
        layerBoundaryCount = 0;

    std::size_t
        invalidCommandCount = 0;

    void Reset()
    {
        *this = {};
    }
};

class RenderQueue
{
public:
    void Clear();

    void Submit(
        const Sprite& sprite,
        const Transform& transform
    );

    void Sort();

    void Execute(
        SpriteRenderer& renderer
    );

    const RenderBatchStats&
        GetBatchStats() const
    {
        return m_batchStats;
    }

    std::size_t GetCommandCount() const;

private:
    std::vector<SpriteRenderCommand>
        m_commands;

    RenderBatchStats m_batchStats;

    std::uint64_t
        m_submissionCounter = 0;
};