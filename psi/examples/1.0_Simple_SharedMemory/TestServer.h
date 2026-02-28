
#pragma once

#include "ITest.h"
#include "psi/shared/ipc/server/IEventServerIPC.h"
#include "psi/shared/ipc/server/IServerIPC.h"
#include "psi/thread/ILoop.h"

namespace psi::examples {

class TestServer : public ITest, public ipc::server::IServerIPC<TestServer>
{
public:
    TestServer(std::shared_ptr<thread::ILoop> loop)
        : ipc::server::IServerIPC<TestServer>("TestService", loop)
        , m_void_event(*this)
        , m_complex_event(*this)
    {
    }

    TestServer &server() override
    {
        return *this;
    }

    ipc::server::FnServiceMap<TestServer> generateFnMap() override
    {
        return {
            {uint16_t(1), &TestServer::callNoArgsNoCb},
            {uint16_t(2), &TestServer::callArgsNoCb},
            {uint16_t(3), &TestServer::callNoArgsVoidCb},
            {uint16_t(4), &TestServer::callNoArgsComplexCb},
            {uint16_t(5), &TestServer::callArgsCb},
            {uint16_t(6), &TestServer::stringCallArgs},
            // {uint16_t(7), &TestServer::voidEvent},
            // {uint16_t(8), &TestServer::complexEvent},
        };
    }

    void callNoArgsNoCb() override
    {
        /// NOT IMPLEMENTED
    }

    void callArgsNoCb(double, bool, std::string, int8_t) override
    {
        /// NOT IMPLEMENTED
    }

    void callNoArgsVoidCb(VoidCb) override
    {
        /// NOT IMPLEMENTED
    }

    void callNoArgsComplexCb(ComplexCb) override
    {
        /// NOT IMPLEMENTED
    }

    void callArgsCb(long double, bool, std::string, uint64_t, ComplexCb) override
    {
        /// NOT IMPLEMENTED
    }

    std::string stringCallArgs(long double, bool, std::string, uint16_t) override
    {
        /// NOT IMPLEMENTED
        return std::string();
    }

    VoidEv &voidEvent() override
    {
        /// NOT IMPLEMENTED
        return m_void_event;
    }

    ComplexEv &complexEvent() override
    {
        /// NOT IMPLEMENTED
        return m_complex_event;
    }

    void notify_VoidEvent()
    {
        m_void_event.notify();
    }

    void notify_ComplexEvent(double a, bool b, std::string c, int32_t d)
    {
        m_complex_event.notify(a, b, c, d);
    }

protected:
    ipc::server::IEventServerIPC<EV_VOID> m_void_event;
    ipc::server::IEventServerIPC<EV_COMPLEX, double, bool, std::string, int32_t> m_complex_event;
};

} // namespace psi::examples
