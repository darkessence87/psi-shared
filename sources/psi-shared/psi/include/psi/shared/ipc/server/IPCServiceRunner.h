#pragma once

#include "IServerIPC.h"

#include <atomic>
#include <thread>

namespace psi {

class ILoop;

namespace ipc {

template <typename T>
class ServiceRunner
{
public:
    ServiceRunner(std::shared_ptr<ILoop> loop)
        : m_loop(loop)
    {
    }

    virtual ~ServiceRunner()
    {
        stop();
    }

    void run(const std::string &serviceName)
    {
        m_service = std::make_unique<T>(serviceName, m_loop);
        m_service->setFnMap();
        m_service->run();
    }

    void stop()
    {
        m_service.reset();
    }

    std::shared_ptr<ILoop> m_loop;
    std::unique_ptr<server::IServerIPC<T>> m_service;
};

} // namespace ipc
} // namespace psi