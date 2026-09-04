#include "EventBus.h"

#include <utility>


EventSubscription::
EventSubscription(
    std::function<void()> disconnect)
    :
    m_disconnect(
        std::move(disconnect)
    )
{
}


EventSubscription::
EventSubscription(
    EventSubscription&& other
) noexcept
    :
    m_disconnect(
        std::move(
            other.m_disconnect
        )
    )
{
    other.m_disconnect = {};
}


EventSubscription::
~EventSubscription()
{
    Reset();
}


EventSubscription&
EventSubscription::operator=(
    EventSubscription&& other
    ) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    Reset();

    m_disconnect =
        std::move(
            other.m_disconnect
        );

    other.m_disconnect = {};

    return *this;
}


void EventSubscription::Reset()
noexcept
{
    if (!m_disconnect)
    {
        return;
    }

    auto disconnect =
        std::move(
            m_disconnect
        );

    m_disconnect = {};

    disconnect();
}