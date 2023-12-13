#pragma once

#include <functional>
#include <stdint.h>
#include <string>

namespace psi::examples {

class ITest
{
public:
    virtual ~ITest() = default;

    using VoidCb = std::function<void()>;
    using ComplexCb = std::function<void(double, bool, std::string, int32_t)>;

    virtual void callNoArgsNoCb() = 0;
    virtual void callArgsNoCb(double, bool, std::string, int8_t) = 0;
    virtual void callNoArgsVoidCb(VoidCb) = 0;
    virtual void callNoArgsComplexCb(ComplexCb) = 0;
    virtual void callArgsCb(long double, bool, std::string, uint64_t, ComplexCb) = 0;
    virtual std::string stringCallArgs(long double, bool, std::string, uint16_t) = 0;
};

} // namespace psi::examples