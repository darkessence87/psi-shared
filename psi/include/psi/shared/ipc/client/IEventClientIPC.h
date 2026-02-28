#pragma once

#include "psi/shared/ipc/IPCEvent.h"
#include "psi/shared/ipc/client/IClientIPC.hpp"

namespace psi::ipc::client {

template <uint16_t EventId, typename... Args>
class IEventClientIPC : public IPCEvent<EventId, Args...>
{
public:
    using Interface = IPCEvent<EventId, Args...>;
    using OnEventUpdateFn = typename Interface::OnEventUpdateFn;
    struct Listener;
    using WeakSubscription = std::weak_ptr<Listener>;

    ///
    /// Listener will automatically unsubscribe from event if it is destroyed
    ///
    struct Listener final : std::enable_shared_from_this<Listener>, comm::Subscribable {
        using Identifier = typename std::list<WeakSubscription>::iterator;

        Listener(IClientIPC &cl, std::list<WeakSubscription> &holder, uint16_t subId)
            : m_process(cl)
            , m_holder(holder)
            , m_subscriptionId(subId)
        {
        }

        ~Listener() override
        {
            m_process.unsubscribeFromEventUpdates(m_subscriptionId);
            m_holder.erase(m_identifier);
        }

    private:
        IClientIPC &m_process;
        std::list<WeakSubscription> &m_holder;
        Identifier m_identifier;
        uint16_t m_subscriptionId;

        friend class IEventClientIPC<EventId, Args...>;
    };

    IEventClientIPC(IClientIPC &cl)
        : m_client(cl)
    {
    }

public: /// IPCEvent implementation
    comm::Subscription subscribe(OnEventUpdateFn &&fn) override
    {
        std::function<void(Args...)> f = std::forward<OnEventUpdateFn>(fn);
        auto clientId = m_client.subscribeToEventUpdates<Args...>(Interface::s_event_id, f);

        if (!clientId.has_value()) {
            return nullptr;
        }

        auto listener = std::make_shared<Listener>(m_client, m_listeners, clientId.value());
        m_listeners.emplace_front(listener);
        listener->m_identifier = m_listeners.begin();
        return std::dynamic_pointer_cast<comm::Subscribable>(listener);
    }

private:
    IClientIPC &m_client;
    std::list<WeakSubscription> m_listeners;
};

} // namespace psi::ipc::client
