#pragma once

#include <iostream>
#include <map>

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

    virtual ~IServerIPC() override = default;

    virtual T &server() = 0;
    virtual FnServiceMap<T> generateFnMap() = 0;

    void run(std::chrono::microseconds sleepTime = std::chrono::microseconds(1))
    {
        m_fnServiceMap = server().generateFnMap();
        static_cast<IServerIPCBase*>(this)->run(sleepTime);
    }

private:
    void onReadCallMemory(uint8_t *queue, uint32_t queueSz) override
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        for (uint32_t i = 0; i < queueSz;) {
            if (i + IPCCall::HEAD_SZ > queueSz) {
                break;
            }
            IPCCall ipcCall;
            ipcCall.deserialize(queue, i);
            i += IPCCall::HEAD_SZ;

            if (i + sizeof(uint16_t) > queueSz) {
                break;
            }
            uint16_t callSz;
            std::memcpy(&callSz, queue + i, sizeof(uint16_t));
            i += sizeof(uint16_t);

            if (i + callSz > queueSz) {
                break;
            }
            if (auto fnItr = m_fnServiceMap.find(ipcCall.m_method_id); fnItr != m_fnServiceMap.end()) {
                auto fn = fnItr->second;
                // make fn data buffer
                auto callData = std::shared_ptr<uint8_t[]>(new uint8_t[callSz]);
                std::memcpy(callData.get(), queue + i, callSz);
                if (m_loop) {
                    m_loop->invoke(m_guard.invoke([this, fn, callData, cbIndex = ipcCall.m_cb_index]() {
                        // std::cout << callData.asHexString() << std::endl;
                        fn(server(), callData.get(), m_guard.invoke([this, cbIndex](auto... args) {
                            this->onCallback(cbIndex, args...);
                        }));
                    }));
                } else {
                    fn(server(), callData.get(), m_guard.invoke([this, cbIndex = ipcCall.m_cb_index](auto... args) {
                        this->onCallback(cbIndex, args...);
                    }));
                }
            } else {
                std::cerr << "Unsupported method: " << tools::to_hex_string(ipcCall.m_method_id) << std::endl;
            }

            i += callSz;
        }
#pragma clang diagnostic pop
    }

private:
    FnServiceMap<T> m_fnServiceMap;
    std::shared_ptr<thread::ILoop> m_loop;
    comm::SafeCaller m_guard;
};

} // namespace psi::ipc::server
