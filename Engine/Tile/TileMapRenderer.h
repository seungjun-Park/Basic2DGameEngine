#pragma once

#include "Engine/Components/Sprite.h"
#include "Engine/Components/Transform.h"

#include <cstddef>
#include <vector>

class TileMap;
class RenderQueue;

class TileMapRenderer
{
public:
    TileMapRenderer() = default;
    ~TileMapRenderer() = default;

    TileMapRenderer(
        const TileMapRenderer&
    ) = delete;

    TileMapRenderer&
        operator=(
            const TileMapRenderer&
            ) = delete;

    bool Build(
        const TileMap& tileMap
    );

    void Clear();

    void Submit(
        RenderQueue& renderQueue
    ) const;

    std::size_t
        GetRenderItemCount() const
    {
        return
            m_renderItems.size();
    }

private:
    struct TileRenderItem
    {
        Sprite sprite;
        Transform transform;
    };

private:
    std::vector<TileRenderItem>
        m_renderItems;
};