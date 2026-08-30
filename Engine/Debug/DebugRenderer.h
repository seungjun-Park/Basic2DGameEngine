#pragma once

#include <DirectXMath.h>

class SpriteRenderer;
class Texture;

class DebugRenderer
{
public:
    bool Initialize(
        Texture* whiteTexture
    );

    void DrawRect(
        SpriteRenderer& renderer,
        const DirectX::XMFLOAT2& center,
        const DirectX::XMFLOAT2& size,
        float thickness = 2.0f
    );

private:
    Texture* m_whiteTexture = nullptr;
};