#pragma once

#include "RenderCommand.h"

#include <DirectXMath.h>

#include <cstddef>
#include <cstdint>
#include <vector>

struct Sprite;
struct Transform;

class Texture;
class SpriteRenderer;

class RenderQueue
{
public:
    void Clear();

    // Entity Sprite¿ë
    void Submit(
        const Sprite& sprite,
        const Transform& transform
    );

    // Tile / procedural sprite¿ë
    void Submit(
        Texture* texture,
        const DirectX::XMFLOAT2& position,
        const DirectX::XMFLOAT2& scale,
        float rotation,
        const UVRect& uv,
        RenderLayer layer,
        float sortZ,
        BlendMode blendMode
    );

    void Sort();

    void Execute(
        SpriteRenderer& renderer
    ) const;

    std::size_t
        GetCommandCount() const;

private:
    std::vector<SpriteRenderCommand>
        m_commands;

    std::uint64_t
        m_submissionCounter = 0;
};