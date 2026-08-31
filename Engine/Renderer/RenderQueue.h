#pragma once

#include "RenderCommand.h"

#include <cstddef>
#include <cstdint>
#include <vector>

struct Sprite;
struct Transform;

class SpriteRenderer;

class RenderQueue
{
public:
    void Clear();

    void Submit(
        const Sprite& sprite,
        const Transform& transform
    );

    void Sort();

    void Execute(
        SpriteRenderer& renderer
    ) const;

    std::size_t GetCommandCount() const;

private:
    std::vector<SpriteRenderCommand>
        m_commands;

    std::uint64_t
        m_submissionCounter = 0;
};