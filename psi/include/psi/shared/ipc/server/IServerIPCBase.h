#pragma once

#include <chrono>
#include <thread>

#include "psi/shared/ipc/protocol/Serializer.h"
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

#include "psi/shared/i_sm_managers.h"

namespace psi::ipc::server {

class IServerIPCBase
{
    template <typename C>
    using ServiceMemoryPtr = std::shared_ptr<i_typed_sm_manager<C>>;

    using CallMemoryPtr = ServiceMemoryPtr<CallSpace<>>;
    using CbMemoryPtr = ServiceMemoryPtr<CallbackSpace<>>;
    using EvMemoryPtr = ServiceMemoryPtr<EventSpace<>>;

public:
    IServerIPCBase(const std::string &serviceName);
    virtual ~IServerIPCBase();

    void run(std::chrono::microseconds = std::chrono::microseconds(1));

protected:
    virtual void onReadCallMemory(uint8_t *, uint32_t) = 0;
    void onCallback(std::optional<uint16_t> cbIndex, const uint8_t *cbData, uint16_t cbLen);

    template <typename... Args>
    void notifyEvent(uint16_t event_id, Args... args)
    {
        uint8_t data[512] = {};
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
        std::shared_ptr<i_sm_manager> mgr = nullptr;
        if constexpr (std::is_same_v<C, CallSpace<>>) {
            mgr = i_sm_managers::create_CallSpace(name);
        }
        if constexpr (std::is_same_v<C, CallbackSpace<>>) {
            mgr = i_sm_managers::create_CallbackSpace(name);
        }
        if constexpr (std::is_same_v<C, EventSpace<>>) {
            mgr = i_sm_managers::create_EventSpace(name);
        }

        auto mem = get_sm_object<C>(mgr);

        if (mgr->isShared()) {
            mgr->loadFromShared();
            mem->lock();
            mem->read()->setAvailable(true);
            mem->unlock();
            std::cout << "[IServerIPCBase] Initialized service [" << mgr->name() << "] for all connected clients"
                      << std::endl;
        } else {
            static C space = C();
            space.setAvailable(true);
            mgr->loadToShared(&space, sizeof(C));
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
