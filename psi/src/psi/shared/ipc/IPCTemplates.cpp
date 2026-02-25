
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"
#include "psi/shared/sm_manager_impl.h"
#include "psi/shared/sm_object_impl.h"

namespace psi {

template class sm_object_impl<ipc::CallSpace<>>;
template class sm_object_impl<ipc::CallbackSpace<>>;
template class sm_object_impl<ipc::EventSpace<>>;

template class sm_manager_impl<ipc::CallSpace<>>;
template class sm_manager_impl<ipc::CallbackSpace<>>;
template class sm_manager_impl<ipc::EventSpace<>>;

} // namespace psi
