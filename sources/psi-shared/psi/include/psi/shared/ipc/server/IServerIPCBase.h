#pragma once

#include <chrono>
#include <thread>

#include "psi/shared/ISharedMemoryManager.hpp"
#include "psi/shared/Serializer.h"
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"

namespace psi::ipc::server {

class IServerIPCBase
{
    template <typename T>
    using ServiceMemory = ISharedMemoryManager<T, SynchType::InterProcess>;

    template <typename T>
    using ServiceMemoryPtr = std::shared_ptr<ServiceMemory<T>>;

    using CallMemoryPtr = ServiceMemoryPtr<CallSpace<>>;
    using CbMemoryPtr = ServiceMemoryPtr<CallbackSpace<>>;
    using EvMemoryPtr = ServiceMemoryPtr<EventSpace<>>;

public:
    IServerIPCBase(const std::string &serviceName);
    virtual ~IServerIPCBase();

    void run(std::chrono::microseconds = std::chrono::microseconds(1));

protected:
    virtual void onReadCallMemory(uint8_t *, size_t) = 0;
    void onCallback(std::optional<size_t> cbIndex, const uint8_t *cbData, size_t cbLen);

    template <typename... Args>
    std::optional<size_t> registerEvent(const std::string &evName)
    {
        m_evMemory->getSharedMemory()->lock();
        const auto id = m_evMemory->getSharedMemory()->read()->registerEvent(evName.c_str());
        m_evMemory->getSharedMemory()->unlock();
        return id;
    }

    template <typename... Args>
    void notifyEvent(size_t evIndex, Args... args)
    {
        uint8_t data[512] = {};
        const auto n = serializer::serializeType(data, args...);
        m_evMemory->getSharedMemory()->lock();
        m_evMemory->getSharedMemory()->read()->push(evIndex, data, n);
        m_evMemory->getSharedMemory()->unlock();
    }

private:
    template <typename T>
    ServiceMemoryPtr<T> allocateMemory(const std::string &name)
    {
        auto mem = ServiceMemory<T>::getInstance(name);
        if (mem->isShared()) {
            mem->loadFromShared();
            mem->getSharedMemory()->lock();
            mem->getSharedMemory()->read()->setAvailable(true);
            mem->getSharedMemory()->unlock();
            std::cout << "[IServerIPCBase] Initialized service [" << mem->name() << "] for all connected clients" << std::endl;
        } else {
            static T space = T();
            space.setAvailable(true);
            mem->loadToShared(&space);
            std::cout << "[IServerIPCBase] Created service [" << mem->name() << "]" << std::endl;
        }
        return mem;
    }

    template <typename T>
    void freeMemory(ServiceMemoryPtr<T> mem)
    {
        if (!mem) {
            std::cout << "[IServerIPCBase] service is not created" << std::endl;
            return;
        }

        auto ptr = mem->getSharedMemory();
        ptr->lock();
        auto obj = ptr->read();
        obj->setAvailable(false);
        ptr->unlock();

        std::cout << "[IServerIPCBase] Destroyed service [" << mem->name() << "]" << std::endl;
    }

    void readMemory();

private:
    bool m_isActive = false;
    std::thread m_serverThread;

    CallMemoryPtr m_callMemory;
    CbMemoryPtr m_cbMemory;
    EvMemoryPtr m_evMemory;

private:
    template <typename... Args>
    friend class IEventServerIPC;
};

} // namespace psi::ipc::server
