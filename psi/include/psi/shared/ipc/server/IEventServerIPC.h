
#pragma once

#include <optional>

#include "IServerIPC.h"
#include "psi/comm/Subscription.h"
#include "psi/shared/ipc/IPCEvent.h"

namespace psi::ipc::server {

template <uint16_t EventId, typename... Args>
class IEventServerIPC : public IPCEvent<EventId, Args...>
{
    using Interface = IPCEvent<EventId, Args...>;

public:
    IEventServerIPC(IServerIPCBase &s)
        : server(s)
    {
    }

public: /// IPCEvent implementation
    comm::Subscription subscribe(Interface::OnEventUpdateFn &&) override
    {
        return nullptr;
    }

public:
    void notify(Args... args)
    {
        server.notifyEvent(Interface::s_event_id, args...);
    }

private:
    IServerIPCBase &server;
};

} // namespace psi::ipc::server
