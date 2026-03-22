
#ifndef PSI_SHARED_IPC_CONFIG_H
#define PSI_SHARED_IPC_CONFIG_H

#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

namespace psi::ipc {

struct IPCConfig_Default {
    using User_CallSpace = CallSpace_Default;
    using User_CallbackSpace = CallbackSpace_Default;
    using User_EventSpace = EventSpace_Default;
};

} // namespace psi::ipc

#ifndef PSI_IPC_CONFIG
#define PSI_IPC_CONFIG psi::ipc::IPCConfig_Default
#endif

namespace psi::ipc {
using IPCConfig = PSI_IPC_CONFIG;
}

#endif
