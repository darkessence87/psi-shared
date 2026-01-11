#pragma once

#include <chrono>
#include <condition_variable>
#include <future>
#include <list>

#include "psi/comm/Attribute.h"
#include "psi/comm/SafeCaller.h"
#include "psi/shared/Deserializer.h"
#include "psi/shared/ISharedMemoryManager.hpp"
#include "psi/shared/Serializer.h"
#include "psi/shared/ipc/IPCCall.h"
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"
#include "psi/thread/ILoop.h"

#include "TemplateHelpers.h"
#include "Types.h"

namespace psi::ipc::client {

template <typename T>
using ServiceMemory = ISharedMemoryManager<T, SynchType::InterProcess>;

template <typename T>
using ServiceMemoryPtr = std::shared_ptr<ServiceMemory<T>>;

template <typename T>
using SharedMemory = IBaseSharedMemoryObject<T, SynchType::InterProcess>;

template <typename T>
using SharedMemoryPtr = std::shared_ptr<SharedMemory<T>>;

#define INVOKE_SERVER_FN(methodId, ...) callServer(psi::ipc::client::CallStruct {methodId, __VA_ARGS__})
#define INVOKE_SERVER_FN_RETURN(returnType, methodId, ...) callServer<returnType>(psi::ipc::client::CallStruct {methodId, __VA_ARGS__})

class IClientIPC
{
public:
    using IsServerAvailableAttribute = comm::Attribute<bool>;

    IClientIPC(const std::string &name, std::shared_ptr<psi::thread::ILoop>);
    virtual ~IClientIPC();

    void connect();
    void disconnect();

    IsServerAvailableAttribute::Interface &isServerAvailableAttribute();

protected:
    template <typename... A>
    void callServer(CallStruct<A...> args, int timeout = 10000)
    {
        auto callArgs = std::apply(
            [](auto... ts) {
                auto tt = std::tuple_cat(std::conditional_t<(std::is_trivially_copyable<decltype(ts)>::value
                                                             || std::is_same<std::string, decltype(ts)>::value),
                                                            std::tuple<decltype(ts)>,
                                                            std::tuple<>> {}...);
                copy_tuple(std::tuple(ts...), tt);
                return tt;
            },
            std::tuple<A...>(args.args));
        auto cbType = std::apply(
            [](auto... ts) {
                auto tt = std::tuple_cat(std::conditional_t<(!std::is_trivially_copyable<decltype(ts)>::value
                                                             && !std::is_same<std::string, decltype(ts)>::value),
                                                            std::tuple<decltype(ts)>,
                                                            std::tuple<>> {}...);
                if constexpr (std::tuple_size_v<decltype(tt)> != 0) {
                    std::get<0>(tt) = std::get<sizeof...(A) - 1u>(std::tuple(ts...));
                }
                return tt;
            },
            std::tuple<A...>(args.args));

        static size_t callId = 0;
        ++callId;

        // case 1: no args, no callback = empty call
        if constexpr (std::tuple_size_v<decltype(callArgs)> == 0 && std::tuple_size_v<decltype(cbType)> == 0) {
            pushCallWithNoArgs(args.methodId, callId, 0);
        }
        // case 2: args, no callback
        else if constexpr (std::tuple_size_v<decltype(cbType)> == 0) {
            pushCallWithArgs(callArgs, args.methodId, callId);
        }
        // case 3: no args, callback
        else if constexpr (std::tuple_size_v<decltype(callArgs)> == 0) {
            auto cbIndex = pushCb(args.methodId, callId, std::get<0>(cbType), timeout);
            pushCallWithNoArgs(args.methodId, callId, cbIndex);
        }
        // case 4: args, callback
        else {
            auto cbIndex = pushCb(args.methodId, callId, std::get<0>(cbType), timeout);
            pushCallWithArgs(callArgs, args.methodId, callId, cbIndex);
        }
    }

    template <typename R, typename... A>
    R callServer(CallStruct<A...> args)
    {
        std::promise<R> p;
        std::future<R> r = p.get_future();
        auto resultCb = [&p](R result) { p.set_value(result); };

        using ReturnFn = std::function<void(R)>;
        callServer(CallStruct<A..., ReturnFn>(args.methodId, std::tuple_cat(args.args, std::tuple<ReturnFn>(resultCb))));

        return r.get();
    }

    template <typename... Args>
    std::optional<size_t> subscribeToEventUpdates(const std::string &evName, std::function<void(Args...)> onNotify)
    {
        m_evMemory->lock();
        auto evIndex = m_evMemory->read()->eventId(evName.c_str());
        auto evClientId = m_evMemory->read()->registerClient();
        m_evMemory->unlock();

        if (!evIndex.has_value()) {
            std::cerr << "Event [" << evName << "] is not registered or unsupported!" << std::endl;
            return std::nullopt;
        }

        if (!evClientId.has_value()) {
            std::cerr << "Event [" << evName << "] can not be subscribed. Too many clients!" << std::endl;
            return std::nullopt;
        }

        std::unique_ptr<EventInfo> evInfo = std::make_unique<EventInfo>(*this);
        evInfo->evIndex = evIndex.value();
        evInfo->m_evClientId = evClientId;

        // evSize
        if constexpr (sizeof...(Args) == 0) {
            evInfo->evSize = 0;
        } else {
            fnPerTuple<0u>(
                [&](const auto &arg) -> int {
                    evInfo->evSize += serializer::sizeOfArgs(arg);
                    return 1;
                },
                std::tuple<Args...>());
        }

        // evNotification
        evInfo->ev = [evIndex, onNotify](uint8_t *data, size_t dataSz) {
            auto fnArgTypes = exportFunctionArgTypes(onNotify);
            if (data == nullptr) {
                std::apply(onNotify, fnArgTypes);
            } else {
                if constexpr (std::tuple_size_v<decltype(fnArgTypes)> == 0) {
                    onNotify();
                } else {
                    size_t offset = 0;
                    while (offset < dataSz) {
                        fnPerTuple<0u>(
                            [&](auto &arg) -> int {
                                deserializer::deserializeTuple(arg, data, offset);
                                return 1;
                            },
                            fnArgTypes);
                        std::apply(onNotify, fnArgTypes);
                    }
                    delete[] data;
                }
            }
        };

        std::unique_lock<std::mutex> lock(m_mtxEvData);
        m_evDataListeners.emplace_back(std::move(evInfo));
        m_conditionEvData.notify_one();

        return evClientId;
    }

    void unsubscribeFromEventUpdates(size_t clientId);

private:
    template <typename T>
    SharedMemoryPtr<T> connectToService(const std::string &name)
    {
        auto mem = ServiceMemory<T>::getInstance(name);
        if (mem->isShared()) {
            std::cout << "[IClientIPC] Connected to existing service [" << mem->name() << "]" << std::endl;
            mem->loadFromShared();
        } else {
            static T space = T();
            mem->loadToShared(&space);
            std::cout << "[IClientIPC] Connected to new service [" << mem->name() << "]" << std::endl;
        }

        return mem->getSharedMemory();
    }

    void pushCallWithNoArgs(uint16_t methodId, size_t callId, size_t cbIndex)
    {
        uint8_t serializedCall[512] = {'\0'};
        IPCCall callInfo(methodId, m_clientId, callId, cbIndex);
        const auto n = serializer::serializeType(serializedCall, callInfo);
        serializer::serializeType(serializedCall + n, uint16_t(0));
        m_callMemory->lock();
        m_callMemory->read()->push(serializedCall, 2 + n);
        m_callMemory->unlock();
    };

    template <typename... A>
    void pushCallWithArgs(const std::tuple<A...> &callArgs,
                          uint16_t methodId,
                          size_t callId,
                          std::optional<size_t> cbIndex = {})
    {
        uint8_t serializedCall[512] = {'\0'};
        IPCCall callInfo(methodId, m_clientId, callId, cbIndex.has_value() ? cbIndex.value() : 0);
        auto n = serializer::serializeType(serializedCall, callInfo);

        size_t argsSize = 0;
        fnPerTuple<0u>(
            [&](const auto &arg) -> int {
                argsSize += serializer::sizeOfArgs(arg);
                return 1;
            },
            callArgs);

        if (argsSize > 512 - 2 - n) {
            std::cerr << "Could not process"
                      << " methodId: " << methodId << ", clientId: " << m_clientId << ", callId: " << callId
                      << " as call arguments are too big (" << argsSize << ")!" << std::endl;
            return;
        }
        uint8_t call[512] = {'\0'};
        size_t callSz = 0;
        fnPerTuple<0u>(
            [&](const auto &arg) -> int {
                serializer::serializeTuple(call, arg, callSz);
                return 1;
            },
            callArgs);

        serializer::serializeType(serializedCall + n, static_cast<uint16_t>(callSz));
        memcpy(serializedCall + 2 + n, call, callSz);
        m_callMemory->lock();
        m_callMemory->read()->push(serializedCall, 2 + n + callSz);
        m_callMemory->unlock();
    }

    template <typename CbType>
    size_t pushCb(uint16_t methodId, size_t callId, CbType cb, int timeout)
    {
        m_cbMemory->lock();
        const size_t cbIndex = m_cbMemory->read()->pushCallback();
        m_cbMemory->unlock();

        if (!cbIndex) {
            return cbIndex;
        }

        CallbackInfo cbInfo;
        cbInfo.timeout = std::chrono::high_resolution_clock().now() + std::chrono::milliseconds(timeout);
        cbInfo.cbIndex = cbIndex;
        cbInfo.fn = [=, this](uint8_t *data) {
            auto fnArgTypes = exportFunctionArgTypes(cb);
            if (data == nullptr) {
                std::cerr << "methodId: " << methodId << ", clientId: " << m_clientId << ", callId: " << callId
                          << " callback timeout!" << std::endl;
                m_cbMemory->lock();
                m_cbMemory->read()->clearCallback(cbInfo.cbIndex);
                m_cbMemory->unlock();
                std::apply(cb, fnArgTypes);
            } else {
                if constexpr (std::tuple_size_v<decltype(fnArgTypes)> == 0) {
                    cb();
                } else {
                    size_t offset = 0;
                    fnPerTuple<0u>(
                        [&](auto &arg) -> int {
                            deserializer::deserializeTuple(arg, data, offset);
                            return 1;
                        },
                        fnArgTypes);
                    delete[] data;
                    std::apply(cb, fnArgTypes);
                }
            }
        };

        std::unique_lock<std::mutex> lock(m_mtxCbData);
        m_cbDataSubs.emplace_back(std::make_unique<CallbackInfo>(cbInfo));
        m_conditionCbData.notify_one();

        return cbIndex;
    }

    bool isConnectionStatusChanged() const;
    void updateConnectionStatus();

    void readCbData();
    void readEvData();
    void readConnectionData();

private:
    std::shared_ptr<psi::thread::ILoop> m_loop;
    comm::SafeCaller m_guard;
    std::atomic<bool> m_isActive = false;
    std::atomic<bool> m_isTrackingConnection = false;
    uint16_t m_clientId = 0;

    SharedMemoryPtr<CallSpace<>> m_callMemory;
    SharedMemoryPtr<CallbackSpace<>> m_cbMemory;
    SharedMemoryPtr<EventSpace<>> m_evMemory;

    std::thread m_cbDataThread;
    std::thread m_evDataThread;
    std::thread m_connectionDataThread;

    std::mutex m_mtxCbData;
    std::mutex m_mtxEvData;
    std::mutex m_mtxConnection;

    std::condition_variable m_conditionCbData;
    std::condition_variable m_conditionEvData;
    std::condition_variable m_conditionConnection;

    IsServerAvailableAttribute m_isServerAvailableAttribute;

    struct CallbackInfo final {
        using Callback = std::function<void(uint8_t *)>;
        size_t cbIndex = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> timeout;
        Callback fn;
    };
    std::list<std::unique_ptr<CallbackInfo>> m_cbDataSubs;

    struct EventInfo final {
        using Event = std::function<void(uint8_t *, size_t)>;

        EventInfo(IClientIPC &processClient)
            : m_processClient(processClient)
        {
        }

        ~EventInfo()
        {
            if (m_evClientId.has_value()) {
                m_processClient.m_evMemory->lock();
                m_processClient.m_evMemory->read()->unregisterClient(m_evClientId.value());
                m_processClient.m_evMemory->unlock();
            }
        }

        IClientIPC &m_processClient;
        size_t evIndex = 0;
        size_t evSize = 0;
        std::optional<size_t> m_evClientId;
        Event ev;
    };
    std::list<std::unique_ptr<EventInfo>> m_evDataListeners;

    template <typename... Args>
    friend class IEventClientIPC;

    friend struct EventInfo;
};

} // namespace psi::ipc::client
