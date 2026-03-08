#pragma once

#include "ITest.h"
#include "psi/shared/ipc/client/IClientIPC.hpp"
#include "psi/shared/ipc/client/IEventClientIPC.h"
#include "psi/thread/ILoop.h"

namespace psi::examples {

class TestClient : public ITest, public ipc::client::IClientIPC
{
public:
    TestClient(std::shared_ptr<thread::ILoop> loop = nullptr)
        : ipc::client::IClientIPC("TestService", loop)
        , m_void_event(*this)
        , m_complex_event(*this)
        , m_vector_event(*this)
        , m_vector_string_event(*this)
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

    void callVectorCb(VectorCb cb) override
    {
        INVOKE_SERVER_FN(uint16_t(6), cb);
    }

    void callVectorStringCb(VectorStringCb cb) override
    {
        INVOKE_SERVER_FN(uint16_t(7), cb);
    }

    std::string stringCallArgs(long double arg0, bool arg1, std::string arg2, uint16_t arg3) override
    {
        return INVOKE_SERVER_FN_RETURN<std::string>(uint16_t(8), arg0, arg1, arg2, arg3);
    }

    std::vector<std::string> vectorCallArgs() override
    {
        return INVOKE_SERVER_FN_RETURN<std::vector<std::string>>(uint16_t(8));
    }

    VoidEv &voidEvent() override
    {
        return m_void_event;
    }

    ComplexEv &complexEvent() override
    {
        return m_complex_event;
    }

    VectorEv &vectorEvent() override
    {
        return m_vector_event;
    }

    VectorStringEv &vectorStringEvent() override
    {
        return m_vector_string_event;
    }

private:
    ipc::client::IEventClientIPC<EV_VOID> m_void_event;
    ipc::client::IEventClientIPC<EV_COMPLEX, double, bool, std::string, int32_t> m_complex_event;
    ipc::client::IEventClientIPC<EV_VECTOR, std::vector<uint64_t>> m_vector_event;
    ipc::client::IEventClientIPC<EV_VECTOR_STRING, std::vector<std::string>> m_vector_string_event;
};

} // namespace psi::examples
