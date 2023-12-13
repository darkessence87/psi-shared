#include "psi/shared/BaseSharedMemoryObject.hpp"
#include "psi/shared/SharedMemoryManager.hpp"
#include "psi/shared/SharedMemoryObject.hpp"
#include "psi/shared/SynchType.h"
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

namespace psi {

template class IBaseSharedMemoryObject<ipc::CallSpace<>, SynchType::InterProcess>;
template class IBaseSharedMemoryObject<ipc::CallbackSpace<>, SynchType::InterProcess>;
template class IBaseSharedMemoryObject<ipc::EventSpace<>, SynchType::InterProcess>;

template class ISharedMemoryManager<ipc::CallSpace<>, SynchType::InterProcess>;
template class ISharedMemoryManager<ipc::CallbackSpace<>, SynchType::InterProcess>;
template class ISharedMemoryManager<ipc::EventSpace<>, SynchType::InterProcess>;

} // namespace psi