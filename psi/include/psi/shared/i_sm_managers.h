
#pragma once

#include "i_sm_manager.h"

#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

namespace psi {

class i_sm_managers
{
public:
    template <typename Space>
    static std::shared_ptr<i_sm_manager> create(const std::string &name) = delete;

    // CallSpace

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallSpace_Default>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallSpace_Q_1024_D_1024>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallSpace_Q_1024_D_2048>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallSpace_Q_2048_D_512>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallSpace_Q_2048_D_1024>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallSpace_Q_2048_D_2048>(const std::string &);

    // CallbackSpace

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallbackSpace_Default>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallbackSpace_Q_1024_D_1024>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallbackSpace_Q_1024_D_2048>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallbackSpace_Q_2048_D_512>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallbackSpace_Q_2048_D_1024>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::CallbackSpace_Q_2048_D_2048>(const std::string &);

    // EventSpace

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::EventSpace_Default>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::EventSpace_C_32>(const std::string &);

    template <>
    std::shared_ptr<i_sm_manager> create<ipc::EventSpace_C_64>(const std::string &);
};

} // namespace psi
