#include "TileMapRenderer.h"

#include "TileMap.h"
#include "Tileset.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/RenderQueue.h"

bool TileMapRenderer::Build(
    const TileMap& tileMap)
{
    Clear();

    const int mapWidth =
        tileMap.GetWidth();

    const int mapHeight =
        tileMap.GetHeight();

    const int tileWidth =
        tileMap.GetTileWidth();

    const int tileHeight =
        tileMap.GetTileHeight();

    if (mapWidth <= 0 ||
        mapHeight <= 0)
    {
        return false;
    }

    if (tileWidth <= 0 ||
        tileHeight <= 0)
    {
        return false;
    }

    Tileset* tileset =
        tileMap.GetTileset();

    if (!tileset)
    {
        return false;
    }

    Texture* texture =
        tileset->GetTexture();

    if (!texture)
    {
        return false;
    }

    //
    // Stage 4에서는 전체 맵을 미리 생성한다.
    //
    // 아직 Camera Culling을 하지 않으므로
    // 모든 Render Layer의 non-empty tile이
    // m_renderItems에 들어간다.
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

        //
        // Collision layer는
        // 렌더링 대상이 아니다.
        //

        if (layer->type !=
            TileLayerType::Render)
        {
            continue;
        }

        if (!layer->visible)
        {
            continue;
        }

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

                //
                // TileId 0 = Empty
                //

                if (tileId ==
                    InvalidTileId)
                {
                    continue;
                }

                //
                // Stage 2에서 추가한
                // Tileset validation을
                // 여기에서도 방어적으로 사용.
                //

                if (!tileset->
                    IsValidTileId(
                        tileId
                    ))
                {
                    continue;
                }

                TileRenderItem item;

                //
                // Sprite
                //

                item.sprite.texture =
                    texture;

                item.sprite.visible =
                    true;

                item.sprite.layer =
                    layer->renderLayer;

                //
                // 같은 RenderLayer를 사용하는
                // 여러 TileLayer가 있을 경우
                // JSON layer 순서를 유지한다.
                //
                // Stage 4에서는 단순히
                // layerIndex를 zIndex로 사용한다.
                //

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
                // position = Sprite 좌상단
                // scale    = 실제 렌더 크기
                //

                item.transform.position =
                {
                    static_cast<float>(
                        x * tileWidth
                    ),

                    static_cast<float>(
                        y * tileHeight
                    )
                };

                item.transform.scale =
                {
                    static_cast<float>(
                        tileWidth
                    ),

                    static_cast<float>(
                        tileHeight
                    )
                };

                item.transform.rotation =
                    0.0f;

                m_renderItems.emplace_back(
                    item
                );
            }
        }
    }

    return true;
}

void TileMapRenderer::Clear()
{
    m_renderItems.clear();
}

void TileMapRenderer::Submit(
    RenderQueue& renderQueue) const
{
    //
    // RenderQueue는 Sprite/Transform 포인터를
    // command 내부에 저장한다.
    //
    // m_renderItems는 Execute가 끝날 때까지
    // 변경되지 않으므로 이 포인터들은 유효하다.
    //

    for (const auto& item :
        m_renderItems)
    {
        renderQueue.Submit(
            item.sprite,
            item.transform
        );
    }
}