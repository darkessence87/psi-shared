
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
        , m_vector_event(*this)
        , m_vector_string_event(*this)
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
            {uint16_t(6), &TestServer::callVectorCb},
            {uint16_t(7), &TestServer::callVectorStringCb},
            {uint16_t(8), &TestServer::stringCallArgs},
            {uint16_t(9), &TestServer::vectorCallArgs},
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

    void callVectorCb(VectorCb) override
    {
        /// NOT IMPLEMENTED
    }

    void callVectorStringCb(VectorStringCb) override
    {
        /// NOT IMPLEMENTED
    }

    std::string stringCallArgs(long double, bool, std::string, uint16_t) override
    {
        /// NOT IMPLEMENTED
        return std::string();
    }

    std::vector<std::string> vectorCallArgs() override
    {
        /// NOT IMPLEMENTED
        return {};
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

    void notify_VoidEvent()
    {
        m_void_event.notify();
    }

    void notify_ComplexEvent(double a, bool b, std::string c, int32_t d)
    {
        m_complex_event.notify(a, b, c, d);
    }

    void notify_VectorEvent(std::vector<uint64_t> a)
    {
        m_vector_event.notify(a);
    }

    void notify_VectorStringEvent(std::vector<std::string> a)
    {
        m_vector_string_event.notify(a);
    }

protected:
    ipc::server::IEventServerIPC<EV_VOID> m_void_event;
    ipc::server::IEventServerIPC<EV_COMPLEX, double, bool, std::string, int32_t> m_complex_event;
    ipc::server::IEventServerIPC<EV_VECTOR, std::vector<uint64_t>> m_vector_event;
    ipc::server::IEventServerIPC<EV_VECTOR_STRING, std::vector<std::string>> m_vector_string_event;
};

} // namespace psi::examples
