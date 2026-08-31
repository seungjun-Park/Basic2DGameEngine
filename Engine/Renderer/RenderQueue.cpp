#include "RenderQueue.h"

#include "SpriteRenderer.h"

#include "Engine/Components/Sprite.h"
#include "Engine/Components/Transform.h"

#include <algorithm>

void RenderQueue::Clear()
{
    m_commands.clear();
    m_submissionCounter = 0;
}

void RenderQueue::Submit(
    const Sprite& sprite,
    const Transform& transform)
{
    if (!sprite.visible)
    {
        return;
    }

    if (!sprite.texture)
    {
        return;
    }

    SpriteRenderCommand command;

    command.texture =
        sprite.texture;

    command.position =
        transform.position;

    command.scale =
        transform.scale;

    command.rotation =
        transform.rotation;

    command.uv =
        sprite.uv;

    command.layer =
        sprite.layer;

    command.blendMode =
        sprite.blendMode;

    command.sortZ =
        sprite.useYSort
        ? transform.position.y
        : sprite.zIndex;

    command.submissionOrder =
        m_submissionCounter++;

    m_commands.emplace_back(
        command
    );
}

void RenderQueue::Submit(
    Texture* texture,
    const DirectX::XMFLOAT2& position,
    const DirectX::XMFLOAT2& scale,
    float rotation,
    const UVRect& uv,
    RenderLayer layer,
    float sortZ,
    BlendMode blendMode)
{
    if (!texture)
    {
        return;
    }

    SpriteRenderCommand command;

    command.texture =
        texture;

    command.position =
        position;

    command.scale =
        scale;

    command.rotation =
        rotation;

    command.uv =
        uv;

    command.layer =
        layer;

    command.blendMode =
        blendMode;

    command.sortZ =
        sortZ;

    command.submissionOrder =
        m_submissionCounter++;

    m_commands.emplace_back(
        command
    );
}


void RenderQueue::Sort()
{
    std::stable_sort(
        m_commands.begin(),
        m_commands.end(),
        [](
            const SpriteRenderCommand& a,
            const SpriteRenderCommand& b)
        {
            const auto layerA =
                static_cast<std::int32_t>(
                    a.layer
                    );

            const auto layerB =
                static_cast<std::int32_t>(
                    b.layer
                    );

            if (layerA != layerB)
            {
                return
                    layerA <
                    layerB;
            }

            if (a.sortZ !=
                b.sortZ)
            {
                return
                    a.sortZ <
                    b.sortZ;
            }

            return
                a.submissionOrder <
                b.submissionOrder;
        }
    );
}

void RenderQueue::Execute(
    SpriteRenderer& renderer) const
{
    for (const auto& command :
        m_commands)
    {
        renderer.SetBlendMode(
            command.blendMode
        );

        renderer.Draw(
            command
        );
    }
}

std::size_t
RenderQueue::GetCommandCount() const
{
    return
        m_commands.size();
}