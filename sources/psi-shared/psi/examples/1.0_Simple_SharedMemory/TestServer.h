#pragma once

#include "ITest.h"
#include "psi/shared/ipc/server/IServerIPC.h"
#include "psi/thread/ILoop.h"

namespace psi::examples {

class TestServer : public ITest, public ipc::server::IServerIPC<TestServer>
{
public:
    TestServer(std::shared_ptr<thread::ILoop> loop)
        : ipc::server::IServerIPC<TestServer>("TestService", loop)
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
};

} // namespace psi::examples