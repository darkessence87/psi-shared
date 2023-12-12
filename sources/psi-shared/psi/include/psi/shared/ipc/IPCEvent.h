#pragma once

#include <functional>

#include "psi/comm/Subscription.h"

namespace psi::ipc {

template <typename... Args>
class IPCEvent
{
public:
    virtual ~IPCEvent() = default;

    using OnEventUpdateFn = std::function<void(Args...)>;
    virtual Subscription subscribe(OnEventUpdateFn &&) = 0;
};

} // namespace psi::ipc
