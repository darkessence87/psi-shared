#pragma once

#include <chrono>
#include <thread>

#include "psi/shared/ipc/protocol/Serializer.h"
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

#include "psi/shared/i_sm_managers.h"

#include "psi/shared/ipc_config.h"

namespace psi::ipc::server {

class IServerIPCBase
{
    template <typename C>
    using ServiceMemoryPtr = std::shared_ptr<i_typed_sm_manager<C>>;

    using CallMemoryPtr = ServiceMemoryPtr<IPCConfig::User_CallSpace>;
    using CbMemoryPtr = ServiceMemoryPtr<IPCConfig::User_CallbackSpace>;
    using EvMemoryPtr = ServiceMemoryPtr<IPCConfig::User_EventSpace>;

public:
    IServerIPCBase(const std::string &serviceName)
        : m_callMemory(allocateMemory<IPCConfig::User_CallSpace>(serviceName))
        , m_cbMemory(allocateMemory<IPCConfig::User_CallbackSpace>(serviceName))
        , m_evMemory(allocateMemory<IPCConfig::User_EventSpace>(serviceName))
    {
    }
    virtual ~IServerIPCBase();

    void run(std::chrono::microseconds = std::chrono::microseconds(1));

protected:
    virtual void onReadCallMemory(uint8_t *, uint32_t) = 0;
    void onCallback(std::optional<uint16_t> cbIndex, const uint8_t *cbData, uint16_t cbLen);

    template <typename... Args>
    void notifyEvent(uint16_t event_id, Args... args)
    {
        constexpr size_t DATA_SIZE = IPCConfig::User_EventSpace::EVENT_SZ;
        uint8_t data[DATA_SIZE] = {};
        const auto n = serializer::serializeType(data, args...);
        auto mem = m_evMemory->getSharedMemory();
        mem->lock();
        mem->read()->push(event_id, data, n);
        mem->unlock();
    }

private:
    template <typename C>
    std::shared_ptr<i_sm_object<C>> get_sm_object(const std::shared_ptr<i_sm_manager> &base)
    {
        auto typed = std::dynamic_pointer_cast<i_typed_sm_manager<C>>(base);
        return typed ? typed->getSharedMemory() : nullptr;
    }

    template <typename C>
    ServiceMemoryPtr<C> allocateMemory(const std::string &name)
    {
        auto mgr = i_sm_managers::create<C>(name);

        if (mgr->isShared()) {
            mgr->loadFromShared();
            auto mem = get_sm_object<C>(mgr);
            mem->lock();
            mem->read()->setAvailable(true);
            mem->unlock();
            std::cout << "[IServerIPCBase] Initialized service [" << mgr->name() << "] for all connected clients"
                      << std::endl;
        } else {
            auto space = std::make_unique<C>();
            space->setAvailable(true);
            mgr->loadToShared(space.get(), sizeof(C));
            std::cout << "[IServerIPCBase] Created service [" << mgr->name() << "]" << std::endl;
        }

        return std::dynamic_pointer_cast<i_typed_sm_manager<C>>(mgr);
    }

    template <typename C>
    void freeMemory(ServiceMemoryPtr<C> mgr)
    {
        if (!mgr) {
            std::cout << "[IServerIPCBase] service is not created" << std::endl;
            return;
        }

        auto mem = get_sm_object<C>(mgr);
        mem->lock();
        auto obj = mem->read();
        obj->setAvailable(false);
        mem->unlock();

        std::cout << "[IServerIPCBase] Destroyed service [" << mgr->name() << "]" << std::endl;
    }

    void readMemory();

private:
    bool m_isActive = false;
    std::thread m_serverThread;

    CallMemoryPtr m_callMemory;
    CbMemoryPtr m_cbMemory;
    EvMemoryPtr m_evMemory;

private:
    template <uint16_t EventId, typename... Args>
    friend class IEventServerIPC;
};

} // namespace psi::ipc::server
