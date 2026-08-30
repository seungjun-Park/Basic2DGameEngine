#pragma once

class Texture;

struct Sprite
{
    Texture* texture = nullptr;

    bool visible = true;

    int layer = 0;

    DirectX::XMFLOAT2 uvMin
    {
        0.0f,
        0.0f
    };

    DirectX::XMFLOAT2 uvMax
    {
        1.0f,
        1.0f
    };
};