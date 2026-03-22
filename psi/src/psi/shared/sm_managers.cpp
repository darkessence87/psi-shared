
#include "psi/shared/i_sm_managers.h"

#include "sm_manager_impl.h"

namespace psi {

// CallbackSpace

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallbackSpace_Default>(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace_Default>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallbackSpace_Q_1024_D_1024>(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace_Q_1024_D_1024>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallbackSpace_Q_1024_D_2048>(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace_Q_1024_D_2048>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallbackSpace_Q_2048_D_512>(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace_Q_2048_D_512>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallbackSpace_Q_2048_D_1024>(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace_Q_2048_D_1024>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallbackSpace_Q_2048_D_2048>(const std::string &name)
{
    return sm_manager_impl<ipc::CallbackSpace_Q_2048_D_2048>::create(name);
}

// CallSpace

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallSpace_Default>(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace_Default>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallSpace_Q_1024_D_1024>(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace_Q_1024_D_1024>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallSpace_Q_1024_D_2048>(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace_Q_1024_D_2048>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallSpace_Q_2048_D_512>(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace_Q_2048_D_512>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallSpace_Q_2048_D_1024>(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace_Q_2048_D_1024>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::CallSpace_Q_2048_D_2048>(const std::string &name)
{
    return sm_manager_impl<ipc::CallSpace_Q_2048_D_2048>::create(name);
}

// EventSpace

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::EventSpace_Default>(const std::string &name)
{
    return sm_manager_impl<ipc::EventSpace_Default>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::EventSpace_C_32>(const std::string &name)
{
    return sm_manager_impl<ipc::EventSpace_C_32>::create(name);
}

template <>
std::shared_ptr<i_sm_manager> i_sm_managers::create<ipc::EventSpace_C_64>(const std::string &name)
{
    return sm_manager_impl<ipc::EventSpace_C_64>::create(name);
}

} // namespace psi
