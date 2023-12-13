#include "psi/shared/ipc/client/IClientIPC.hpp"

namespace psi::ipc::client {

IClientIPC::IClientIPC(const std::string &name, std::shared_ptr<psi::thread::ILoop> loop)
    : m_loop(loop)
    , m_guard(this)
    , m_callMemory(connectToService<CallSpace<>>(name))
    , m_cbMemory(connectToService<CallbackSpace<>>(name))
    , m_evMemory(connectToService<EventSpace<>>(name))
{
    readConnectionData();
}

IClientIPC::~IClientIPC()
{
    disconnect();

    m_isTrackingConnection = false;
    m_conditionConnection.notify_one();
    if (m_connectionDataThread.joinable()) {
        m_connectionDataThread.join();
    }
}

void IClientIPC::connect()
{
    if (m_isActive) {
        return;
    }

    m_callMemory->lock();
    m_clientId = m_callMemory->read()->generateClientId();
    m_callMemory->unlock();

    m_isActive = true;

    readCbData();
    readEvData();
}

void IClientIPC::disconnect()
{
    if (!m_isActive) {
        return;
    }

    m_isActive = false;

    m_conditionCbData.notify_one();
    if (m_cbDataThread.joinable()) {
        m_cbDataThread.join();
    }

    m_conditionEvData.notify_one();
    if (m_evDataThread.joinable()) {
        m_evDataThread.join();
    }
}

IClientIPC::IsServerAvailableAttribute::Interface &IClientIPC::isServerAvailableAttribute()
{
    return m_isServerAvailableAttribute;
}

void IClientIPC::unsubscribeFromEventUpdates(size_t clientId)
{
    m_evMemory->lock();
    m_evMemory->read()->unregisterClient(clientId);
    m_evMemory->unlock();
}

bool IClientIPC::isConnectionStatusChanged() const
{
    if (m_callMemory->read()->isAvailable() != m_isServerAvailableAttribute.value()) {
        return true;
    }

    if (m_cbMemory->read()->isAvailable() != m_isServerAvailableAttribute.value()) {
        return true;
    }

    if (m_evMemory->read()->isAvailable() != m_isServerAvailableAttribute.value()) {
        return true;
    }

    return false;
}

void IClientIPC::updateConnectionStatus()
{
    if (isConnectionStatusChanged()) {
        const bool cond1 = m_callMemory->read()->isAvailable();
        const bool cond2 = m_cbMemory->read()->isAvailable();
        const bool cond3 = m_evMemory->read()->isAvailable();
        m_isServerAvailableAttribute.setValue(cond1 && cond2 && cond3);
    }
}

void IClientIPC::readCbData()
{
    m_cbDataThread = std::thread([this]() {
        while (m_isActive) {
            std::unique_lock<std::mutex> lock(m_mtxCbData);
            m_conditionCbData.wait(lock, [this]() { return !m_cbDataSubs.empty() || !m_isActive; });

            if (m_cbDataSubs.empty()) {
                continue;
            }

            std::vector<std::function<void()>> callbacks;
            for (auto itr = m_cbDataSubs.begin(); itr != m_cbDataSubs.end();) {
                if (itr->get()->timeout < std::chrono::high_resolution_clock().now() || !m_isActive) {
                    callbacks.emplace_back(std::bind(itr->get()->fn, nullptr));
                    itr = m_cbDataSubs.erase(itr);
                } else {
                    auto obj = m_cbMemory->read();
                    if (obj->isCallbackAvailable(itr->get()->cbIndex)) {
                        m_cbMemory->lock();
                        obj = m_cbMemory->read();
                        uint8_t *data = obj->popCallback(itr->get()->cbIndex);
                        m_cbMemory->unlock();

                        callbacks.emplace_back(std::bind(itr->get()->fn, data));
                        itr = m_cbDataSubs.erase(itr);
                    } else {
                        ++itr;
                    }
                }
            }

            lock.unlock();

            for (auto cb : callbacks) {
                if (m_loop) {
                    m_loop->invoke([cb]() { cb(); });
                } else {
                    cb();
                }
            }
        }
    });
}

void IClientIPC::readEvData()
{
    m_evDataThread = std::thread([this]() {
        while (m_isActive) {
            std::unique_lock<std::mutex> lock(m_mtxEvData);
            m_conditionEvData.wait_for(lock, std::chrono::microseconds(1), [this]() {
                return !m_evDataListeners.empty() || !m_isActive;
            });

            if (m_evDataListeners.empty()) {
                continue;
            }

            std::vector<std::function<void()>> events;
            for (auto itr = m_evDataListeners.begin(); itr != m_evDataListeners.end();) {
                const auto &evInfo = itr->get();
                // process events
                auto evObj = m_evMemory->read();
                if (evObj->isDataAvailable(evInfo->m_evClientId.value(), evInfo->evIndex)) {
                    m_evMemory->lock();
                    evObj = m_evMemory->read();

                    size_t queueSz = 0;
                    uint8_t *queue = evObj->pop(evInfo->m_evClientId.value(), evInfo->evIndex, queueSz);
                    m_evMemory->unlock();

                    events.emplace_back(std::bind(evInfo->ev, queue, queueSz));
                }

                ++itr;
            }

            lock.unlock();

            for (auto ev : events) {
                if (m_loop) {
                    m_loop->invoke([ev]() { ev(); });
                } else {
                    ev();
                }
            }
        }
    });
}

void IClientIPC::readConnectionData()
{
    updateConnectionStatus();

    m_isTrackingConnection = true;
    m_connectionDataThread = std::thread([this]() {
        while (m_isTrackingConnection) {
            updateConnectionStatus();

            std::unique_lock<std::mutex> lock(m_mtxConnection);
            m_conditionConnection.wait_for(lock, std::chrono::seconds(1), [this]() {
                return !m_isTrackingConnection || isConnectionStatusChanged();
            });
        }

        m_isServerAvailableAttribute.setValue(false);
    });
}

} // namespace psi::ipc::client
