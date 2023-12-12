#pragma once

#include <optional>

#include "IServerIPC.h"
#include "psi/comm/Subscription.h"
#include "common/shared/ipc/IPCEvent.h"

namespace psi::ipc::server {

template <typename... Args>
class IEventServerIPC : public IPCEvent<Args...>
{
public:
    IEventServerIPC(IServerIPCBase &s, const std::string &evName)
        : server(s)
        , m_evName(evName)
    {
        if (!m_evIndex) {
            m_evIndex = server.registerEvent<Args...>(m_evName);
        }
    }

    virtual ~IEventServerIPC() {}

public: /// IPCEvent implementation
    Subscription subscribe(IPCEvent<Args...>::OnEventUpdateFn &&) override
    {
        return nullptr;
    }

public:
    void notify(Args... args)
    {
        if (!m_evIndex) {
            m_evIndex = server.registerEvent<Args...>(m_evName);
        }

        if (m_evIndex.has_value()) {
            server.notifyEvent(m_evIndex.value(), args...);
        }
    }

private:
    IServerIPCBase &server;
    std::optional<size_t> m_evIndex;
    const std::string m_evName;
};

} // namespace psi::ipc
