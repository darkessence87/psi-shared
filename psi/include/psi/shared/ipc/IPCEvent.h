
#pragma once

#include <functional>

#include "psi/comm/Subscription.h"

namespace psi::ipc {

template <uint16_t EventId, typename... Args>
class IPCEvent
{
public:
    virtual ~IPCEvent() = default;

    static constexpr uint16_t s_event_id = EventId;

    using OnEventUpdateFn = std::function<void(Args...)>;
    virtual comm::Subscription subscribe(OnEventUpdateFn &&) = 0;
};

} // namespace psi::ipc
