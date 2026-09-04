#include "TileMapCollider.h"

#include "TileMap.h"
#include "TileTypes.h"

#include "Engine/Physics/PhysicsSystem.h"
#include "Engine/Physics/PhysicsUnits.h"

#include <cstdint>
#include <vector>
#include <cassert>

TileMapCollider::~TileMapCollider()
{
    Destroy();
}

bool TileMapCollider::Build(
    const TileMap& tileMap,
    PhysicsSystem& physics)
{
    Destroy();

    if (!physics.IsInitialized())
    {
        return false;
    }

    if (tileMap.GetWidth() <= 0 ||
        tileMap.GetHeight() <= 0)
    {
        return false;
    }

    if (tileMap.GetTileWidth() <= 0 ||
        tileMap.GetTileHeight() <= 0)
    {
        return false;
    }

    const b2WorldId worldId =
        physics.GetWorldId();

    if (!b2World_IsValid(worldId))
    {
        return false;
    }

    m_worldId = worldId;

    m_collisionRects =
        BuildMergedRectangles(tileMap);

    m_mergedTileArea = 0;

    for (const CollisionRect& rect :
        m_collisionRects)
    {
        if (rect.width <= 0 ||
            rect.height <= 0)
        {
            Destroy();

            return false;
        }

        m_mergedTileArea +=
            static_cast<std::size_t>(
                rect.width
                )
            *
            static_cast<std::size_t>(
                rect.height
                );
    }

    //
    // Collision layer가 없거나
    // solid tile이 하나도 없는 map도
    // 유효한 TileMap으로 취급한다.
    //

    if (m_collisionRects.empty())
    {
#ifdef _DEBUG

        assert(
            m_solidTileCount == 0
        );

        assert(
            m_mergedTileArea == 0
        );

        assert(
            m_shapeIds.empty()
        );

        assert(
            !HasBody()
        );
#endif

        return true;
    }

    //
    // TileMap 전체에 static body 하나.
    //
    // Body origin은 World (0,0).
    // 각 rectangle은 offset polygon으로 만든다.
    //

    b2BodyDef bodyDef =
        b2DefaultBodyDef();

    bodyDef.type =
        b2_staticBody;

    bodyDef.position =
    {
        0.0f,
        0.0f
    };

    //
    // TileMap은 Entity가 아니므로
    // Entity userData를 넣지 않는다.
    //

    bodyDef.userData =
        nullptr;

    m_bodyId =
        b2CreateBody(
            m_worldId,
            &bodyDef
        );

    if (B2_IS_NULL(
        m_bodyId))
    {
        Destroy();

        return false;
    }

    b2ShapeDef shapeDef =
        b2DefaultShapeDef();

    //
    // Top-down wall.
    //
    // Player가 벽을 따라 이동할 때
    // 과도한 friction이 걸리지 않도록 0.
    //

    shapeDef.material.friction =
        0.0f;

    shapeDef.material.restitution =
        0.0f;

    shapeDef.isSensor =
        false;

    //
    // Tile wall은 Entity callback 대상이 아니다.
    // Box2D solver collision 자체에는 영향이 없다.
    //

    shapeDef.enableContactEvents =
        false;

    const float tileWidth =
        static_cast<float>(
            tileMap.GetTileWidth()
            );

    const float tileHeight =
        static_cast<float>(
            tileMap.GetTileHeight()
            );

    m_shapeIds.reserve(
        m_collisionRects.size()
    );

    for (const CollisionRect& rect :
        m_collisionRects)
    {
        const float widthPixels =
            static_cast<float>(
                rect.width
                )
            *
            tileWidth;

        const float heightPixels =
            static_cast<float>(
                rect.height
                )
            *
            tileHeight;

        const float halfWidth =
            PhysicsUnits::ToMeters(
                widthPixels * 0.5f
            );

        const float halfHeight =
            PhysicsUnits::ToMeters(
                heightPixels * 0.5f
            );

        //
        // Tile coordinate는 좌상단 기준.
        //
        // Rectangle center:
        //
        // x * tileWidth + width / 2
        //

        const float centerXPixels =
            static_cast<float>(
                rect.x
                )
            *
            tileWidth
            +
            widthPixels * 0.5f;

        const float centerYPixels =
            static_cast<float>(
                rect.y
                )
            *
            tileHeight
            +
            heightPixels * 0.5f;

        const b2Vec2 center
        {
            PhysicsUnits::ToMeters(
                centerXPixels
            ),

            PhysicsUnits::ToMeters(
                centerYPixels
            )
        };

        const b2Polygon box =
            b2MakeOffsetBox(
                halfWidth,
                halfHeight,
                center,
                b2MakeRot(0.0f)
            );

        const b2ShapeId shapeId =
            b2CreatePolygonShape(
                m_bodyId,
                &shapeDef,
                &box
            );

        if (B2_IS_NULL(
            shapeId))
        {
            Destroy();

            return false;
        }

        m_shapeIds.emplace_back(
            shapeId
        );
    }

#ifdef _DEBUG

    //
    // Rectangle merge는
    // solid cell을 잃거나 중복해서는 안 된다.
    //

    assert(
        m_mergedTileArea ==
        m_solidTileCount
    );

    //
    // Collision rectangle 하나당
    // Box2D polygon shape 하나.
    //

    assert(
        m_shapeIds.size() ==
        m_collisionRects.size()
    );

    assert(
        b2Body_IsValid(m_bodyId)
    );

    const int actualShapeCount =
        b2Body_GetShapeCount(m_bodyId);

    assert(
        actualShapeCount ==
        static_cast<int>(
            m_shapeIds.size()
            )
    );

#endif

    return true;
}

void TileMapCollider::Destroy()
{
    if (B2_IS_NON_NULL(m_bodyId))
    {
        //
        // PhysicsSystem object lifetime에 의존하지 않는다.
        //
        // 저장된 Box2D handle 자체의 validity만 확인한다.
        //
        if (b2World_IsValid(m_worldId) &&
            b2Body_IsValid(m_bodyId))
        {
            b2DestroyBody(
                m_bodyId
            );
        }

        m_bodyId =
            b2_nullBodyId;
    }

    m_shapeIds.clear();
    m_collisionRects.clear();

    m_mergedTileArea = 0;
    m_collisionLayerCount = 0;
    m_solidTileCount = 0;

    m_worldId =
        b2_nullWorldId;
}
std::vector<
    TileMapCollider::CollisionRect
>
TileMapCollider::BuildMergedRectangles(
    const TileMap& tileMap)
{
    const int mapWidth =
        tileMap.GetWidth();

    const int mapHeight =
        tileMap.GetHeight();

    const std::size_t cellCount =
        static_cast<std::size_t>(
            mapWidth
            )
        *
        static_cast<std::size_t>(
            mapHeight
            );

    //
    // 여러 Collision Layer가 있다면
    // union해서 하나의 solid grid로 만든다.
    //

    std::vector<std::uint8_t>
        solid(
            cellCount,
            0
        );

    auto getIndex =
        [mapWidth](
            int x,
            int y)
        {
            return
                static_cast<std::size_t>(
                    y
                    )
                *
                static_cast<std::size_t>(
                    mapWidth
                    )
                +
                static_cast<std::size_t>(
                    x
                    );
        };

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
            TileLayerType::Collision)
        {
            continue;
        }

        ++m_collisionLayerCount;

        //
        // Collision layer의 visible 값은
        // 물리에는 영향을 주지 않는다.
        //

        for (
            int y = 0;
            y < mapHeight;
            ++y)
        {
            for (
                int x = 0;
                x < mapWidth;
                ++x)
            {
                const TileId tileId =
                    tileMap.GetTile(
                        layerIndex,
                        x,
                        y
                    );

                if (!IsSolidCollisionTile(tileId))
                {
                    continue;
                }

                solid[
                    getIndex(x, y)
                ] = 1;
            }
        }
    }

    //
    // unique solid tile count
    //

    for (std::uint8_t value :
    solid)
    {
        if (value != 0)
        {
            ++m_solidTileCount;
        }
    }

    std::vector<CollisionRect>
        rectangles;

    if (m_solidTileCount == 0)
    {
        return rectangles;
    }

    std::vector<std::uint8_t>
        used(
            cellCount,
            0
        );

    //
    // Greedy rectangle merge.
    //
    // 1. 현재 cell에서 오른쪽으로
    //    가능한 최대 width 검색
    //
    // 2. 그 width를 유지하면서
    //    아래쪽으로 최대 height 검색
    //
    // 3. 해당 영역을 used 처리
    //

    for (
        int y = 0;
        y < mapHeight;
        ++y)
    {
        for (
            int x = 0;
            x < mapWidth;
            ++x)
        {
            const std::size_t startIndex =
                getIndex(
                    x,
                    y
                );

            if (solid[startIndex] == 0 ||
                used[startIndex] != 0)
            {
                continue;
            }

            int rectWidth = 0;

            while (
                x + rectWidth <
                mapWidth)
            {
                const std::size_t index =
                    getIndex(
                        x + rectWidth,
                        y
                    );

                if (solid[index] == 0 ||
                    used[index] != 0)
                {
                    break;
                }

                ++rectWidth;
            }

            int rectHeight = 1;

            while (
                y + rectHeight <
                mapHeight)
            {
                bool canGrow =
                    true;

                for (
                    int localX = 0;
                    localX < rectWidth;
                    ++localX)
                {
                    const std::size_t index =
                        getIndex(
                            x + localX,
                            y + rectHeight
                        );

                    if (solid[index] == 0 ||
                        used[index] != 0)
                    {
                        canGrow =
                            false;

                        break;
                    }
                }

                if (!canGrow)
                {
                    break;
                }

                ++rectHeight;
            }

            //
            // Rectangle 영역 사용 처리
            //

            for (
                int localY = 0;
                localY < rectHeight;
                ++localY)
            {
                for (
                    int localX = 0;
                    localX < rectWidth;
                    ++localX)
                {
                    used[
                        getIndex(
                            x + localX,
                            y + localY
                        )
                    ] = 1;
                }
            }

            rectangles.emplace_back(
                CollisionRect
                {
                    x,
                    y,
                    rectWidth,
                    rectHeight
                }
            );
        }
    }

    return rectangles;
}