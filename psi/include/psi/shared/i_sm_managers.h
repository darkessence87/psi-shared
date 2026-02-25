
#pragma once

#include "i_sm_manager.h"

namespace psi {

class i_sm_managers
{
public:
    static std::shared_ptr<i_sm_manager> create_CallSpace(const std::string &name);
    static std::shared_ptr<i_sm_manager> create_CallbackSpace(const std::string &name);
    static std::shared_ptr<i_sm_manager> create_EventSpace(const std::string &name);
};

} // namespace psi
