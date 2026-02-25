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

    auto mem = m_callMemory->getSharedMemory();
    mem->lock();

    auto obj = mem->read();
    uint32_t dataSz = 0;
    uint8_t *data = obj->pop(dataSz);
    if (!data || dataSz == 0) {
        mem->unlock();
        return;
    }

    auto buffer = std::unique_ptr<uint8_t[]>(new uint8_t[dataSz]);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    std::memcpy(buffer.get(), data, dataSz);
#pragma clang diagnostic pop

    obj->clear();
    mem->unlock();

    onReadCallMemory(buffer.get(), dataSz);
}

void IServerIPCBase::onCallback(std::optional<uint16_t> cbIndex, const uint8_t *cbData, uint16_t cbLen)
{
    if (!cbIndex.has_value()) {
        return;
    }

    auto mem = m_cbMemory->getSharedMemory();
    mem->lock();
    mem->read()->updateCallback(cbIndex.value(), cbData, cbLen);
    mem->unlock();
}

} // namespace psi::ipc::server
