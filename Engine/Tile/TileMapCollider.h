#pragma once

#include <box2d/box2d.h>

#include <cstddef>
#include <vector>

class PhysicsSystem;
class TileMap;

class TileMapCollider
{
public:
    struct CollisionRect
    {
        int x = 0;
        int y = 0;

        int width = 0;
        int height = 0;
    };

public:
    TileMapCollider() = default;

    TileMapCollider(
        const TileMapCollider&
    ) = delete;

    ~TileMapCollider();

    TileMapCollider&
        operator=(
            const TileMapCollider&
            ) = delete;

    bool Build(
        const TileMap& tileMap,
        PhysicsSystem& physics
    );

    void Destroy();

    std::size_t
        GetCollisionLayerCount() const
    {
        return
            m_collisionLayerCount;
    }

    std::size_t
        GetSolidTileCount() const
    {
        return
            m_solidTileCount;
    }

    std::size_t
        GetShapeCount() const
    {
        return
            m_shapeIds.size();
    }

    std::size_t
        GetMergedTileArea() const
    {
        return
            m_mergedTileArea;
    }

    const std::vector<CollisionRect>&
        GetCollisionRects() const
    {
        return
            m_collisionRects;
    }

    bool HasBody() const
    {
        return
            B2_IS_NON_NULL(m_bodyId) &&
            b2World_IsValid(m_worldId) &&
            b2Body_IsValid(m_bodyId);
    }

private:
    std::vector<CollisionRect>
        BuildMergedRectangles(
            const TileMap& tileMap
        );

private:
    b2WorldId m_worldId =
        b2_nullWorldId;

    b2BodyId m_bodyId =
        b2_nullBodyId;

    std::vector<b2ShapeId>
        m_shapeIds;

    std::vector<CollisionRect>
        m_collisionRects;

    std::size_t
        m_collisionLayerCount = 0;

    std::size_t
        m_solidTileCount = 0;

    std::size_t
        m_mergedTileArea = 0;
};