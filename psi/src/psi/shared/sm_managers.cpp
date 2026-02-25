
#include "psi/shared/i_sm_managers.h"

#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

#include "sm_manager_impl.h"

namespace psi {

std::shared_ptr<i_sm_manager> i_sm_managers::create_CallSpace(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace<>>::create(name);
}

std::shared_ptr<i_sm_manager> i_sm_managers::create_CallbackSpace(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace<>>::create(name);
}

std::shared_ptr<i_sm_manager> i_sm_managers::create_EventSpace(const std::string &name)
{
    return sm_manager_impl<ipc::EventSpace<>>::create(name);
}

} // namespace psi
