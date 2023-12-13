#pragma once

#include "common/shared/ipc/IPCEvent.h"
#include "common/shared/ipc/client/IClientIPC.hpp"

namespace psi::ipc::client {

template <typename... Args>
class IEventClientIPC : public IPCEvent<Args...>
{
public:
    using Interface = IPCEvent<Args...>;
    using OnEventUpdateFn = typename Interface::OnEventUpdateFn;
    class Listener;
    using WeakSubscription = std::weak_ptr<Listener>;

    ///
    /// Listener will automatically unsubscribe from event if it is destroyed
    ///
    struct Listener final : std::enable_shared_from_this<Listener>, Subscribable {
        using Identifier = typename std::list<WeakSubscription>::iterator;

        Listener(IClientIPC &cl, std::list<WeakSubscription> &holder, size_t subId)
            : m_process(cl)
            , m_holder(holder)
            , m_subscriptionId(subId)
        {
        }

        ~Listener()
        {
            m_process.unsubscribeFromEventUpdates(m_subscriptionId);
            m_holder.erase(m_identifier);
        }

    private:
        IClientIPC &m_process;
        std::list<WeakSubscription> &m_holder;
        Identifier m_identifier;
        size_t m_subscriptionId;

        friend class IEventClientIPC<Args...>;
    };

    IEventClientIPC(IClientIPC &cl, const std::string &evName)
        : m_client(cl)
        , m_evName(evName)
    {
    }

public: /// IPCEvent implementation
    Subscription subscribe(OnEventUpdateFn &&fn) override
    {
        std::function<void(Args...)> f = std::forward<OnEventUpdateFn>(fn);
        auto clientId = m_client.subscribeToEventUpdates<Args...>(m_evName, f);

        if (!clientId.has_value()) {
            return nullptr;
        }

        auto listener = std::make_shared<Listener>(m_client, m_listeners, clientId.value());
        m_listeners.emplace_front(listener);
        listener->m_identifier = m_listeners.begin();
        return std::dynamic_pointer_cast<Subscribable>(listener);
    }

private:
    IClientIPC &m_client;
    const std::string m_evName;
    std::list<WeakSubscription> m_listeners;
};

} // namespace psi::ipc::client
