#include "psi/shared/ipc/server/IServerIPCBase.h"

namespace psi::ipc::server {

IServerIPCBase::IServerIPCBase(const std::string &serviceName)
    : m_callMemory(allocateMemory<CallSpace<>>(serviceName))
    , m_cbMemory(allocateMemory<CallbackSpace<>>(serviceName))
    , m_evMemory(allocateMemory<EventSpace<>>(serviceName))
{
}

IServerIPCBase::~IServerIPCBase()
{
    m_isActive = false;
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }

    freeMemory(m_callMemory);
    freeMemory(m_cbMemory);
    freeMemory(m_evMemory);
}

void IServerIPCBase::run(std::chrono::microseconds sleepTime)
{
    m_isActive = true;

    m_serverThread = std::thread([this, sleepTime]() {
        while (m_isActive) {
            readMemory();
            std::this_thread::sleep_for(sleepTime);
        }
    });
}

void IServerIPCBase::readMemory()
{
    if (!m_callMemory->getSharedMemory()->read()->isDataAvailable()) {
        return;
    }

    m_callMemory->getSharedMemory()->lock();
    auto callObj = m_callMemory->getSharedMemory()->read();
    size_t queueSz = 0;
    uint8_t *queue = callObj->pop(queueSz);
    m_callMemory->getSharedMemory()->unlock();

    onReadCallMemory(queue, queueSz);
    delete[] queue;
}

void IServerIPCBase::onCallback(std::optional<size_t> cbIndex, const uint8_t *cbData, size_t cbLen)
{
    if (!cbIndex.has_value()) {
        return;
    }

    m_cbMemory->getSharedMemory()->lock();
    m_cbMemory->getSharedMemory()->read()->updateCallback(cbIndex.value(), cbData, cbLen);
    m_cbMemory->getSharedMemory()->unlock();
}

} // namespace psi::ipc
