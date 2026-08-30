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

    return m_whiteTexture != nullptr;
}

void DebugRenderer::DrawRect(
    SpriteRenderer& renderer,
    const DirectX::XMFLOAT2& center,
    const DirectX::XMFLOAT2& size,
    float thickness)
{
    if (!m_whiteTexture)
        return;

    Sprite sprite;
    sprite.texture =
        m_whiteTexture;

    Transform top;
    top.position =
    {
        center.x,
        center.y - size.y * 0.5f
    };
    top.scale =
    {
        size.x,
        thickness
    };

    Transform bottom;
    bottom.position =
    {
        center.x,
        center.y + size.y * 0.5f
    };
    bottom.scale =
    {
        size.x,
        thickness
    };

    Transform left;
    left.position =
    {
        center.x - size.x * 0.5f,
        center.y
    };
    left.scale =
    {
        thickness,
        size.y
    };

    Transform right;
    right.position =
    {
        center.x + size.x * 0.5f,
        center.y
    };
    right.scale =
    {
        thickness,
        size.y
    };

    renderer.Draw(sprite, top);
    renderer.Draw(sprite, bottom);
    renderer.Draw(sprite, left);
    renderer.Draw(sprite, right);
}