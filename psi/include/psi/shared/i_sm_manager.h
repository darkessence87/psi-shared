
#pragma once

#include <memory>
#include <string>

#include "i_sm_object.h"

namespace psi {

class i_sm_manager
{
public:
    virtual ~i_sm_manager() = default;
    virtual bool isShared() = 0;
    virtual void loadToShared(const void *const object, size_t object_sz) = 0;
    virtual void *loadFromShared() = 0;
    virtual std::string name() const = 0;
};

template <typename C>
class i_typed_sm_manager : public i_sm_manager
{
public:
    virtual std::shared_ptr<i_sm_object<C>> getSharedMemory() = 0;
};

} // namespace psi
