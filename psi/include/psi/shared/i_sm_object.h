#pragma once

namespace psi {

template <typename C>
class i_sm_object
{
public:
    virtual ~i_sm_object() = default;

    virtual void lock() = 0;
    virtual void unlock() = 0;

    virtual C *read() const = 0;
    virtual bool write(const C *const object) = 0;
};

} // namespace psi
