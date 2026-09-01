#include "DebugRenderer.h"

#include "Engine/Renderer/SpriteRenderer.h"

#include "Engine/Components/Transform.h"
#include "Engine/Components/Sprite.h"

#include "Engine/Graphics/Texture.h"

bool DebugRenderer::Initialize(
    Texture* whiteTexture)
{
    m_whiteTexture =
        whiteTexture;

    return
        m_whiteTexture != nullptr;
}

void DebugRenderer::DrawRect(
    SpriteRenderer& renderer,
    const DirectX::XMFLOAT2& center,
    const DirectX::XMFLOAT2& size,
    float thickness)
{
    if (!m_whiteTexture)
    {
        return;
    }

    if (size.x <= 0.0f ||
        size.y <= 0.0f ||
        thickness <= 0.0f)
    {
        return;
    }

    Sprite sprite;

    sprite.texture =
        m_whiteTexture;

    const float halfWidth =
        size.x * 0.5f;

    const float halfHeight =
        size.y * 0.5f;

    //
    // 현재 Renderer contract:
    //
    // Transform.position = 좌상단
    //
    // DrawRect API:
    //
    // center = 사각형 중심
    //
    // 따라서 실제 line Transform은
    // center에서 좌상단으로 변환해야 한다.
    //

    Transform top;

    top.position =
    {
        center.x - halfWidth,
        center.y - halfHeight
    };

    top.scale =
    {
        size.x,
        thickness
    };


    Transform bottom;

    bottom.position =
    {
        center.x - halfWidth,
        center.y + halfHeight - thickness
    };

    bottom.scale =
    {
        size.x,
        thickness
    };


    Transform left;

    left.position =
    {
        center.x - halfWidth,
        center.y - halfHeight
    };

    left.scale =
    {
        thickness,
        size.y
    };


    Transform right;

    right.position =
    {
        center.x + halfWidth - thickness,
        center.y - halfHeight
    };

    right.scale =
    {
        thickness,
        size.y
    };


    renderer.Draw(
        sprite,
        top
    );

    renderer.Draw(
        sprite,
        bottom
    );

    renderer.Draw(
        sprite,
        left
    );

    renderer.Draw(
        sprite,
        right
    );
}