#include "RenderQueue.h"

#include "SpriteRenderer.h"

#include "Engine/Components/Sprite.h"
#include "Engine/Components/Transform.h"

#include <algorithm>
#include <cassert>

namespace
{
    bool RenderCommandLess(
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
                layerA < layerB;
        }

        if (a.sortZ != b.sortZ)
        {
            return
                a.sortZ < b.sortZ;
        }

        return
            a.submissionOrder <
            b.submissionOrder;
    }
}

void RenderQueue::Clear()
{
    m_commands.clear();

    m_submissionCounter = 0;

    m_batchStats.Reset();
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

    command.texture =
        sprite.texture;

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
        RenderCommandLess
    );

#ifdef _DEBUG

    for (std::size_t i = 1;
        i < m_commands.size();
        ++i)
    {
        //
        // 현재 command가 이전 command보다
        // 앞으로 정렬되어 있으면 안 된다.
        //
        assert(
            !RenderCommandLess(
                m_commands[i],
                m_commands[i - 1]
            )
        );
    }

#endif
}

void RenderQueue::Execute(
    SpriteRenderer& renderer)
{
    m_batchStats.Reset();

    std::size_t commandIndex = 0;

    while (commandIndex <
        m_commands.size())
    {
        const SpriteRenderCommand&
            firstCommand =
            m_commands[
                commandIndex
            ];

        if (!IsSpriteRenderCommandValid(
            firstCommand))
        {
            ++m_batchStats.
                invalidCommandCount;

            ++commandIndex;

            continue;
        }

        std::size_t batchEnd =
            commandIndex + 1;

        while (batchEnd <
            m_commands.size())
        {
            const SpriteRenderCommand&
                nextCommand =
                m_commands[
                    batchEnd
                ];

            if (!IsSpriteRenderCommandValid(
                nextCommand))
            {
                break;
            }

            if (CanBatchSpriteRenderCommands(
                firstCommand,
                nextCommand))
            {
                ++batchEnd;

                continue;
            }

            //
            // 다음 valid command가 현재 batch와
            // 호환되지 않으므로 여기서 batch boundary.
            //
            ++m_batchStats.
                batchBoundaryCount;

            //
            // 한 boundary에서 여러 state가 동시에
            // 바뀔 수 있으므로 각 transition은
            // 독립적으로 계측한다.
            //
            if (nextCommand.texture !=
                firstCommand.texture)
            {
                ++m_batchStats.
                    textureBoundaryCount;
            }

            if (nextCommand.blendMode !=
                firstCommand.blendMode)
            {
                ++m_batchStats.
                    blendBoundaryCount;
            }

            if (nextCommand.layer !=
                firstCommand.layer)
            {
                ++m_batchStats.
                    layerBoundaryCount;
            }

            break;
        }

        const std::size_t batchSize =
            batchEnd -
            commandIndex;

        ++m_batchStats.batchCount;

        m_batchStats.batchedCommandCount +=
            batchSize;

        m_batchStats.maxBatchSize =
            max(
                m_batchStats.maxBatchSize,
                batchSize
            );

        if (batchSize == 1)
        {
            ++m_batchStats.
                singleCommandBatchCount;
        }

        renderer.SetBlendMode(
            firstCommand.blendMode
        );

        renderer.DrawBatch(
            m_commands.data() +
            commandIndex,
            batchSize
        );

        commandIndex =
            batchEnd;
    }

#ifdef _DEBUG

    assert(
        m_batchStats.
        batchedCommandCount +
        m_batchStats.
        invalidCommandCount ==
        m_commands.size()
    );

    assert(
        m_batchStats.batchCount <=
        m_batchStats.batchedCommandCount
    );

    assert(
        m_batchStats.
        singleCommandBatchCount <=
        m_batchStats.batchCount
    );

    if (m_batchStats.
        batchedCommandCount > 0)
    {
        assert(
            m_batchStats.maxBatchSize >=
            1
        );

        assert(
            m_batchStats.maxBatchSize <=
            m_batchStats.
            batchedCommandCount
        );
    }

    //
    // 정상적인 RenderQueue::Submit() 경로에서는
    // invalid command가 만들어져서는 안 된다.
    //
    assert(
        m_batchStats.
        invalidCommandCount == 0
    );

    if (!m_commands.empty())
    {
        //
        // invalid command가 없다면:
        //
        // N logical batch에는 정확히 N - 1개의
        // batch boundary가 존재한다.
        //
        assert(
            m_batchStats.
            batchBoundaryCount + 1 ==
            m_batchStats.batchCount
        );
    }

#endif
}

std::size_t
RenderQueue::GetCommandCount() const
{
    return
        m_commands.size();
}