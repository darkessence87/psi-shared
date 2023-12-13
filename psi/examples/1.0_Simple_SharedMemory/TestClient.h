#pragma once

#include "ITest.h"
#include "psi/shared/ipc/client/IClientIPC.hpp"
#include "psi/thread/ILoop.h"

namespace psi::examples {

class TestClient : public ITest, public ipc::client::IClientIPC
{
public:
    TestClient(std::shared_ptr<thread::ILoop> loop = nullptr)
        : ipc::client::IClientIPC("TestService", loop)
    {
    }

    void callNoArgsNoCb() override
    {
        INVOKE_SERVER_FN(uint16_t(1));
    }

    void callArgsNoCb(double arg0, bool arg1, std::string arg2, int8_t arg3) override
    {
        INVOKE_SERVER_FN(uint16_t(2), arg0, arg1, arg2, arg3);
    }

    void callNoArgsVoidCb(VoidCb arg0) override
    {
        INVOKE_SERVER_FN(uint16_t(3), arg0);
    }

    void callNoArgsComplexCb(ComplexCb arg0) override
    {
        INVOKE_SERVER_FN(uint16_t(4), arg0);
    }

    void callArgsCb(long double arg0, bool arg1, std::string arg2, uint64_t arg3, ComplexCb arg4) override
    {
        INVOKE_SERVER_FN(uint16_t(5), arg0, arg1, arg2, arg3, arg4);
    }

    std::string stringCallArgs(long double arg0, bool arg1, std::string arg2, uint16_t arg3) override
    {
        return INVOKE_SERVER_FN_RETURN(std::string, uint16_t(6), arg0, arg1, arg2, arg3);
    }
};

} // namespace psi::examples