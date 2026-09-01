#pragma once

#include <box2d/box2d.h>

#include <cstddef>
#include <vector>

class PhysicsSystem;
class TileMap;

class TileMapCollider
{
public:
    TileMapCollider() = default;

    ~TileMapCollider();

    TileMapCollider(
        const TileMapCollider&
    ) = delete;

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

    bool HasBody() const
    {
        return
            B2_IS_NON_NULL(
                m_bodyId
            );
    }

private:
    struct CollisionRect
    {
        int x = 0;
        int y = 0;

        int width = 0;
        int height = 0;
    };

private:
    std::vector<CollisionRect>
        BuildMergedRectangles(
            const TileMap& tileMap
        );

private:
    PhysicsSystem* m_physics =
        nullptr;

    b2BodyId m_bodyId =
        b2_nullBodyId;

    std::vector<b2ShapeId>
        m_shapeIds;

    std::size_t
        m_collisionLayerCount = 0;

    std::size_t
        m_solidTileCount = 0;
};