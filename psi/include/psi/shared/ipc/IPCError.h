
#pragma once

#include <stdint.h>

namespace psi::ipc {

enum IPCError : uint16_t
{
    None = 0,
    Timeout = 1,
    TransportFailure = 2,
    InvalidPayload = 3
};

} // namespace psi::ipc
