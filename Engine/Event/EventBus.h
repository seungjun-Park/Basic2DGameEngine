#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

class EventBus;

class EventSubscription final
{
public:

    EventSubscription() = default;

    ~EventSubscription();

    EventSubscription(
        const EventSubscription&
    ) = delete;

    EventSubscription&
        operator=(
            const EventSubscription&
            ) = delete;

    EventSubscription(
        EventSubscription&& other
    ) noexcept;

    EventSubscription&
        operator=(
            EventSubscription&& other
            ) noexcept;

    void Reset() noexcept;

private:

    friend class EventBus;

    explicit EventSubscription(
        std::function<void()> disconnect
    );

private:

    std::function<void()>
        m_disconnect;
};


class EventBus final
{
public:
    EventBus() = default;
    ~EventBus() = default;

    EventBus(
        const EventBus&
    ) = delete;

    EventBus&
        operator=(
            const EventBus&
            ) = delete;

    EventBus(
        EventBus&&
    ) = delete;

    EventBus&
        operator=(
            EventBus&&
            ) = delete;


    template<
        typename TEvent,
        typename THandler
    >
    [[nodiscard]]
    EventSubscription Subscribe(
        THandler&& handler
    )
    {
        using EventType =
            std::remove_cvref_t<
            TEvent
            >;

        using HandlerType =
            std::decay_t<
            THandler
            >;

        static_assert(
            std::is_object_v<
            EventType
            >,
            "Event type must be an object type."
            );

        static_assert(
            std::is_invocable_r_v<
            void,
            HandlerType&,
            const EventType&
            >,
            "Event handler must be callable "
            "with const TEvent&."
            );

        auto channel =
            GetOrCreateChannel<
            EventType
            >();

        typename EventChannel<
            EventType
        >::Handler callback(
            std::forward<
            THandler
            >(handler)
        );

        if (!callback)
        {
            return {};
        }

        const SubscriptionId id =
            channel->Subscribe(
                std::move(callback)
            );

        if (id == InvalidSubscriptionId)
        {
            return {};
        }

        std::weak_ptr<
            EventChannel<EventType>
        > weakChannel =
            channel;

        return EventSubscription(
            [
                weakChannel,
                id
            ]() noexcept
            {
                if (auto locked =
                    weakChannel.lock())
                {
                    locked->Unsubscribe(
                        id
                    );
                }
            }
        );
    }


    template<typename TEvent>
    void Publish(
        const TEvent& event
    )
    {
        using EventType =
            std::remove_cvref_t<
            TEvent
            >;

        auto channel =
            FindChannel<
            EventType
            >();

        if (!channel)
        {
            return;
        }

        channel->Publish(
            event
        );
    }


    template<typename TEvent>
    std::size_t GetSubscriberCount()
        const noexcept
    {
        using EventType =
            std::remove_cvref_t<
            TEvent
            >;

        auto channel =
            FindChannel<
            EventType
            >();

        if (!channel)
        {
            return 0;
        }

        return channel->
            GetSubscriberCount();
    }


private:

    using SubscriptionId =
        std::uint64_t;

    static constexpr
        SubscriptionId
        InvalidSubscriptionId = 0;


    struct IEventChannel
    {
        virtual ~IEventChannel() =
            default;
    };


    template<typename TEvent>
    class EventChannel final
        :
        public IEventChannel
    {
    public:

        using Handler =
            std::function<
            void(
                const TEvent&
                )
            >;


        SubscriptionId Subscribe(
            Handler handler
        )
        {
            if (!handler)
            {
                return
                    InvalidSubscriptionId;
            }

            if (m_nextId ==
                InvalidSubscriptionId)
            {
                return
                    InvalidSubscriptionId;
            }

            const SubscriptionId id =
                m_nextId++;

            m_slots.push_back(
                Slot
                {
                    id,
                    std::move(handler),
                    true
                }
            );

            return id;
        }


        void Unsubscribe(
            SubscriptionId id
        ) noexcept
        {
            if (id ==
                InvalidSubscriptionId)
            {
                return;
            }

            for (Slot& slot :
                m_slots)
            {
                if (slot.id != id)
                {
                    continue;
                }

                if (!slot.active)
                {
                    return;
                }

                slot.active =
                    false;

                if (m_dispatchDepth ==
                    0)
                {
                    Compact();
                }

                return;
            }
        }


        void Publish(
            const TEvent& event
        )
        {
            ++m_dispatchDepth;

            const std::size_t
                dispatchCount =
                m_slots.size();

            try
            {
                for (
                    std::size_t index = 0;
                    index < dispatchCount;
                    ++index
                    )
                {
                    if (!m_slots[index].
                        active)
                    {
                        continue;
                    }

                    Handler callback =
                        m_slots[index].
                        handler;

                    if (callback)
                    {
                        callback(
                            event
                        );
                    }
                }
            }
            catch (...)
            {
                FinishDispatch();

                throw;
            }

            FinishDispatch();
        }


        std::size_t
            GetSubscriberCount()
            const noexcept
        {
            return
                static_cast<
                std::size_t
                >(
                    std::count_if(
                        m_slots.begin(),
                        m_slots.end(),
                        [](
                            const Slot& slot
                            )
                        {
                            return
                                slot.active;
                        }
                    )
                    );
        }


    private:

        struct Slot
        {
            SubscriptionId id =
                InvalidSubscriptionId;

            Handler handler;

            bool active =
                false;
        };


        void FinishDispatch()
            noexcept
        {
            if (m_dispatchDepth == 0)
            {
                return;
            }

            --m_dispatchDepth;

            if (m_dispatchDepth == 0)
            {
                Compact();
            }
        }


        void Compact()
            noexcept
        {
            std::erase_if(
                m_slots,
                [](
                    const Slot& slot
                    )
                {
                    return
                        !slot.active;
                }
            );
        }


    private:

        std::vector<Slot>
            m_slots;

        SubscriptionId
            m_nextId = 1;

        std::uint32_t
            m_dispatchDepth = 0;
    };


    template<typename TEvent>
    std::shared_ptr<
        EventChannel<TEvent>
    >
        GetOrCreateChannel()
    {
        const std::type_index
            type =
            std::type_index(
                typeid(TEvent)
            );

        const auto it =
            m_channels.find(
                type
            );

        if (it !=
            m_channels.end())
        {
            return
                std::static_pointer_cast<
                EventChannel<TEvent>
                >(
                    it->second
                );
        }

        auto channel =
            std::make_shared<
            EventChannel<TEvent>
            >();

        m_channels.emplace(
            type,
            channel
        );

        return channel;
    }


    template<typename TEvent>
    std::shared_ptr<
        EventChannel<TEvent>
    >
        FindChannel()
    {
        const std::type_index
            type =
            std::type_index(
                typeid(TEvent)
            );

        const auto it =
            m_channels.find(
                type
            );

        if (it ==
            m_channels.end())
        {
            return nullptr;
        }

        return
            std::static_pointer_cast<
            EventChannel<TEvent>
            >(
                it->second
            );
    }


    template<typename TEvent>
    std::shared_ptr<
        const EventChannel<TEvent>
    >
        FindChannel()
        const noexcept
    {
        const std::type_index
            type =
            std::type_index(
                typeid(TEvent)
            );

        const auto it =
            m_channels.find(
                type
            );

        if (it ==
            m_channels.end())
        {
            return nullptr;
        }

        return
            std::static_pointer_cast<
            const EventChannel<TEvent>
            >(
                it->second
            );
    }

private:
    std::unordered_map<
        std::type_index,
        std::shared_ptr<
        IEventChannel
        >
    >
        m_channels;
};