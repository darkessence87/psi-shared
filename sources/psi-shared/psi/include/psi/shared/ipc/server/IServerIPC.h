#pragma once

#include <iostream>

#include "IServerIPCBase.h"

#include "psi/comm/SafeCaller.h"
#include "psi/shared/ipc/IPCCall.h"
#include "psi/thread/ILoop.h"
#include "psi/tools/Tools.h"

#include "MemberFnWrapper.h"

namespace psi::ipc::server {

template <typename T>
using FnServiceMap = std::map<uint16_t, MemberFnWrapper<T>>;

template <typename T>
class IServerIPC : public IServerIPCBase
{
public:
    IServerIPC(const std::string &name, std::shared_ptr<thread::ILoop> loop)
        : IServerIPCBase(name)
        , m_loop(loop)
        , m_guard(this)
    {
    }

    virtual ~IServerIPC() = default;

    virtual T &server() = 0;
    virtual FnServiceMap<T> generateFnMap() = 0;

    void run(std::chrono::microseconds sleepTime = std::chrono::microseconds(1))
    {
        m_fnServiceMap = server().generateFnMap();
        ((IServerIPCBase *)this)->run(sleepTime);
    }

private:
    void onReadCallMemory(uint8_t *queue, size_t queueSz) override
    {
        for (size_t i = 0; i < queueSz;) {
            auto deserializedParams =
                deserializer::deserializeTypeVariadic<IPCCall, uint16_t>(queue + i, 0, nullptr, nullptr);
            const auto &ipcCall = std::get<0>(deserializedParams);
            const auto callSz = std::get<1>(deserializedParams);
            constexpr size_t headSz = sizeof(IPCCall) + 2;

            if (auto fnItr = m_fnServiceMap.find(ipcCall.methodId); fnItr != m_fnServiceMap.end()) {
                auto fn = fnItr->second;
                // make fn data buffer
                uint8_t *callData = new uint8_t[callSz]();
                memcpy(callData, queue + i + headSz, callSz);
                if (m_loop) {
                    m_loop->invoke(m_guard.invoke([this, fn, callData, cbIndex = ipcCall.cbIndex]() {
                        // std::cout << callData.asHexString() << std::endl;
                        fn(server(), callData, 0, m_guard.invoke([this, cbIndex](auto... args) {
                            this->onCallback(cbIndex, args...);
                        }));
                        delete[] callData;
                    }));
                } else {
                    fn(server(), callData, 0, m_guard.invoke([this, cbIndex = ipcCall.cbIndex](auto... args) {
                        this->onCallback(cbIndex, args...);
                    }));
                    delete[] callData;
                }
            } else {
                std::cerr << "Unsupported method: " << tools::to_hex_string(ipcCall.methodId) << std::endl;
            }

            i += headSz + callSz;
        }
    }

private:
    FnServiceMap<T> m_fnServiceMap;
    std::shared_ptr<thread::ILoop> m_loop;
    comm::SafeCaller m_guard;
};

} // namespace psi::ipc::server
