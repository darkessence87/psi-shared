
#pragma once

#include <functional>
#include <stdint.h>
#include <string>

#include "psi/shared/ipc/IPCCallback.h"
#include "psi/shared/ipc/IPCEvent.h"

namespace psi::examples {

class ITest
{
public:
    virtual ~ITest() = default;

    using VoidCb = ipc::IPCCallback<>;
    using ComplexCb = ipc::IPCCallback<double, bool, std::string, int32_t>;
    virtual void callNoArgsNoCb() = 0;
    virtual void callArgsNoCb(double, bool, std::string, int8_t) = 0;
    virtual void callNoArgsVoidCb(VoidCb) = 0;
    virtual void callNoArgsComplexCb(ComplexCb) = 0;
    virtual void callArgsCb(long double, bool, std::string, uint64_t, ComplexCb) = 0;
    virtual std::string stringCallArgs(long double, bool, std::string, uint16_t) = 0;

    enum : uint16_t
    {
        EV_VOID = 0,
        EV_COMPLEX = 1,
        EV_COUNT
    };

    using VoidEv = ipc::IPCEvent<EV_VOID>;
    using ComplexEv = ipc::IPCEvent<EV_COMPLEX, double, bool, std::string, int32_t>;
    virtual VoidEv &voidEvent() = 0;
    virtual ComplexEv &complexEvent() = 0;
};

} // namespace psi::examples
