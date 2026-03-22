
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"
#include "psi/shared/sm_manager_impl.h"
#include "psi/shared/sm_object_impl.h"

namespace psi {

// CallSpace
template class sm_object_impl<ipc::CallSpace_Default>;
template class sm_object_impl<ipc::CallSpace_Q_1024_D_1024>;
template class sm_object_impl<ipc::CallSpace_Q_1024_D_2048>;
template class sm_object_impl<ipc::CallSpace_Q_2048_D_512>;
template class sm_object_impl<ipc::CallSpace_Q_2048_D_1024>;
template class sm_object_impl<ipc::CallSpace_Q_2048_D_2048>;

template class sm_manager_impl<ipc::CallSpace_Default>;
template class sm_manager_impl<ipc::CallSpace_Q_1024_D_1024>;
template class sm_manager_impl<ipc::CallSpace_Q_1024_D_2048>;
template class sm_manager_impl<ipc::CallSpace_Q_2048_D_512>;
template class sm_manager_impl<ipc::CallSpace_Q_2048_D_1024>;
template class sm_manager_impl<ipc::CallSpace_Q_2048_D_2048>;

// CallbackSpace
template class sm_object_impl<ipc::CallbackSpace_Default>;
template class sm_object_impl<ipc::CallbackSpace_Q_1024_D_1024>;
template class sm_object_impl<ipc::CallbackSpace_Q_1024_D_2048>;
template class sm_object_impl<ipc::CallbackSpace_Q_2048_D_512>;
template class sm_object_impl<ipc::CallbackSpace_Q_2048_D_1024>;
template class sm_object_impl<ipc::CallbackSpace_Q_2048_D_2048>;

template class sm_manager_impl<ipc::CallbackSpace_Default>;
template class sm_manager_impl<ipc::CallbackSpace_Q_1024_D_1024>;
template class sm_manager_impl<ipc::CallbackSpace_Q_1024_D_2048>;
template class sm_manager_impl<ipc::CallbackSpace_Q_2048_D_512>;
template class sm_manager_impl<ipc::CallbackSpace_Q_2048_D_1024>;
template class sm_manager_impl<ipc::CallbackSpace_Q_2048_D_2048>;

// EventSpace
template class sm_object_impl<ipc::EventSpace_Default>;
template class sm_object_impl<ipc::EventSpace_C_32>;
template class sm_object_impl<ipc::EventSpace_C_64>;

template class sm_manager_impl<ipc::EventSpace_Default>;
template class sm_manager_impl<ipc::EventSpace_C_32>;
template class sm_manager_impl<ipc::EventSpace_C_64>;

} // namespace psi
