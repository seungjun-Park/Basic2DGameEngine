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

    command.sprite =
        &sprite;

    command.transform =
        &transform;

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
    std::size_t commandIndex = 0;

    while (commandIndex <
        m_commands.size())
    {
        const SpriteRenderCommand&
            firstCommand =
            m_commands[
                commandIndex
            ];

        if (!firstCommand.sprite ||
            !firstCommand.transform ||
            !firstCommand.sprite->texture)
        {
            ++commandIndex;
            continue;
        }

        Texture* batchTexture =
            firstCommand.sprite->texture;

        const RenderLayer batchLayer =
            firstCommand.layer;

        const BlendMode batchBlendMode =
            firstCommand.blendMode;

        std::size_t batchEnd =
            commandIndex + 1;

        while (batchEnd <
            m_commands.size())
        {
            const SpriteRenderCommand&
                command =
                m_commands[
                    batchEnd
                ];

            if (!command.sprite ||
                !command.transform ||
                !command.sprite->texture)
            {
                break;
            }

            if (command.layer !=
                batchLayer)
            {
                break;
            }

            if (command.blendMode !=
                batchBlendMode)
            {
                break;
            }

            if (command.sprite->texture !=
                batchTexture)
            {
                break;
            }

            ++batchEnd;
        }

        renderer.SetBlendMode(
            batchBlendMode
        );

        renderer.DrawBatch(
            m_commands.data() +
            commandIndex,
            batchEnd -
            commandIndex
        );

        commandIndex =
            batchEnd;
    }
}

std::size_t
RenderQueue::GetCommandCount() const
{
    return
        m_commands.size();
}