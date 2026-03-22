#include "psi/shared/ipc/client/IClientIPC.hpp"

namespace psi::ipc::client {

IClientIPC::~IClientIPC()
{
    disconnect();

    m_isTrackingConnection = false;
    m_conditionConnection.notify_one();
    if (m_connectionDataThread.joinable()) {
        m_connectionDataThread.join();
    }
}

bool IClientIPC::connect()
{
    if (m_isActive) {
        return true;
    }

    m_callMemory->lock();
    auto clientId = m_callMemory->read()->generateClientId();
    m_callMemory->unlock();

    if (!clientId.has_value()) {
        return false;
    }

    m_clientId = clientId.value();

    m_isActive = true;

    readCbData();
    readEvData();

    return true;
}

void IClientIPC::disconnect()
{
    if (!m_isActive) {
        return;
    }

    m_isActive = false;

    m_conditionCbData.notify_all();
    if (m_cbDataThread.joinable()) {
        m_cbDataThread.join();
    }

    m_conditionEvData.notify_all();
    if (m_evDataThread.joinable()) {
        m_evDataThread.join();
    }
}

IClientIPC::IsServerAvailableAttribute::Interface &IClientIPC::isServerAvailableAttribute()
{
    return m_isServerAvailableAttribute;
}

void IClientIPC::unsubscribeFromEventUpdates(uint16_t clientId)
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
                auto *sub = itr->get();
                if (sub->timeout < std::chrono::high_resolution_clock().now() || !m_isActive) {
                    callbacks.emplace_back([fn = sub->fn]() { fn(nullptr); });
                    itr = m_cbDataSubs.erase(itr);
                } else {
                    std::shared_ptr<uint8_t[]> cb_data;

                    m_cbMemory->lock();
                    auto obj = m_cbMemory->read();
                    if (obj->isCallbackAvailable(sub->cbIndex)) {
                        if (auto cb_view = obj->popCallback(sub->cbIndex); cb_view.has_value()) {
                            cb_data.reset(new uint8_t[cb_view->sz]);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
                            std::memcpy(cb_data.get(), cb_view->data, cb_view->sz);
#pragma clang diagnostic pop
                        }
                    }
                    m_cbMemory->unlock();

                    if (cb_data) {
                        callbacks.emplace_back([fn = sub->fn, cb_data]() { fn(cb_data.get()); });
                        itr = m_cbDataSubs.erase(itr);
                    } else {
                        ++itr;
                    }
                }
            }

            lock.unlock();

            for (auto &cb : callbacks) {
                if (m_loop) {
                    m_loop->invoke([cb_ = std::move(cb)]() { cb_(); });
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
            {
                std::unique_lock<std::mutex> lock(m_mtxEvData);
                m_conditionEvData.wait_for(lock, std::chrono::microseconds(50), [this]() {
                    return (!m_evDataListeners.empty() && m_evClientId.has_value()) || !m_isActive;
                });
            }

            if (!m_isActive || !m_evClientId.has_value()) {
                continue;
            }

            std::vector<std::function<void()>> tasks;
            for (;;) {
                m_evMemory->lock();
                auto event_view = m_evMemory->read()->pop(m_evClientId.value());
                m_evMemory->unlock();

                if (!event_view) {
                    break;
                }

                std::array<uint8_t, MAX_EVENT_SZ> local {};
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
                std::memcpy(local.data(), event_view->data, event_view->sz);
#pragma clang diagnostic pop

                const uint16_t event_id = event_view->event_id;
                const uint16_t sz = event_view->sz;

                std::vector<EventInfo *> targets;
                {
                    std::lock_guard<std::mutex> lock(m_mtxEvData);
                    for (auto &lsn : m_evDataListeners)
                        if (lsn->event_id == event_id)
                            targets.push_back(lsn.get());
                }

                if (targets.empty()) {
                    continue;
                }

                tasks.emplace_back([targets, local, sz]() mutable {
                    for (auto *lsn : targets) {
                        lsn->event_fn(local.data(), sz);
                    }
                });
            }

            for (auto &t : tasks) {
                if (m_loop) {
                    m_loop->invoke(std::move(t));
                } else {
                    t();
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
