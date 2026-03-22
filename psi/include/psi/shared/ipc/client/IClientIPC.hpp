#pragma once

#include <chrono>
#include <condition_variable>
#include <future>
#include <list>

#include "psi/comm/Attribute.h"
#include "psi/comm/SafeCaller.h"
#include "psi/shared/i_sm_managers.h"
#include "psi/shared/ipc/IPCCall.h"
#include "psi/shared/ipc/IPCCallback.h"
#include "psi/shared/ipc/TemplateHelpers.h"
#include "psi/shared/ipc/protocol/Deserializer.h"
#include "psi/shared/ipc/protocol/Serializer.h"
#include "psi/shared/ipc/space/CallSpace.h"
#include "psi/shared/ipc/space/CallbackSpace.h"
#include "psi/shared/ipc/space/EventSpace.h"
#include "psi/thread/ILoop.h"

#include "psi/shared/ipc_config.h"

#include "Types.h"

namespace psi::ipc::client {

template <typename C>
using SharedMemoryPtr = std::shared_ptr<i_sm_object<C>>;

class IClientIPC
{
public:
    using IsServerAvailableAttribute = comm::Attribute<bool>;

    IClientIPC(const std::string &name, std::shared_ptr<psi::thread::ILoop> loop)
        : m_loop(loop)
        , m_guard(this)
        , m_callMemory(connectToService<IPCConfig::User_CallSpace>(name))
        , m_cbMemory(connectToService<IPCConfig::User_CallbackSpace>(name))
        , m_evMemory(connectToService<IPCConfig::User_EventSpace>(name))
    {
        if (!m_evClientId.has_value()) {
            m_evMemory->lock();
            m_evClientId = m_evMemory->read()->registerClient();
            m_evMemory->unlock();
        }

        readConnectionData();
    }

    virtual ~IClientIPC();

    bool connect();
    void disconnect();

    IsServerAvailableAttribute::Interface &isServerAvailableAttribute();

    template <typename... A>
    void INVOKE_SERVER_FN(uint16_t methodId, A &&...args)
    {
        callServer(CallStruct<std::decay_t<A>...>(methodId, std::forward<A>(args)...));
    }

    template <typename R, typename... A>
    R INVOKE_SERVER_FN_RETURN(uint16_t methodId, A &&...args)
    {
        return callServerWithReturn<R>(CallStruct<std::decay_t<A>...>(methodId, std::forward<A>(args)...));
    }

protected:
    template <typename... A>
    void callServer(CallStruct<A...> args, int timeout = 10000)
    {
        static size_t callId = 0;
        ++callId;

        using ArgsTuple = decltype(args.args);
        constexpr size_t N = std::tuple_size_v<ArgsTuple>;

        // case 1: no args, no callback = empty call
        if constexpr (N == 0) {
            pushCallWithNoArgs(args.methodId, callId, 0);
        } else {
            using Last = std::tuple_element_t<N - 1, ArgsTuple>;
            constexpr bool has_cb = is_ipc_callback_v<Last>;

            // case 2: args, no callback
            if constexpr (!has_cb) {
                uint16_t args_sz = 0;
                if (!validateCall(args.args, args_sz)) {
                    fprintf(stderr,
                            "Could not process methodId: %u, clientId: %u, callId: %zu as call arguments are too big "
                            "(%u)!\n",
                            args.methodId,
                            m_clientId,
                            callId,
                            args_sz);
                    return;
                }
                pushCallWithArgs(args.args, args.methodId, callId, 0);
            } else {
                using CallbackT = std::decay_t<std::tuple_element_t<N - 1, ArgsTuple>>;
                CallbackT cb = std::get<N - 1>(args.args);

                // case 3: no args, callback
                if constexpr (N == 1) {
                    auto cbIndex = pushCb(args.methodId, callId, std::move(cb), timeout);
                    if (!cbIndex.has_value()) {
                        const auto error = std::format("Not enough space for callback processing, callId: {}", callId);
                        std::cout << error << std::endl;
                        cb.fail(IPCError::TransportFailure, error);
                        return;
                    }
                    pushCallWithNoArgs(args.methodId, callId, cbIndex.value());
                } else {
                    // case 4: args, callback
                    auto argsWithoutCb = tuple_pop_back(args.args);

                    uint16_t args_sz = 0;
                    if (!validateCall(argsWithoutCb, args_sz)) {
                        const auto error = std::format("Could not process methodId: {}, clientId: {}, callId: {} as "
                                                       "call arguments are too big ({})!",
                                                       args.methodId,
                                                       m_clientId,
                                                       callId,
                                                       args_sz);
                        std::cout << error << std::endl;
                        cb.fail(IPCError::InvalidPayload, error);
                        return;
                    }
                    
                    auto cbIndex = pushCb(args.methodId, callId, std::move(cb), timeout);
                    if (!cbIndex.has_value()) {
                        const auto error = std::format("Not enough space for callback processing, callId: {}", callId);
                        std::cout << error << std::endl;
                        cb.fail(IPCError::TransportFailure, error);
                        return;
                    }
                    pushCallWithArgs(argsWithoutCb, args.methodId, callId, cbIndex.value());
                }
            }
        }
    }

    template <typename R, typename... A>
    R callServerWithReturn(CallStruct<A...> args)
    {
        auto promise = std::make_shared<std::promise<R>>();
        auto future = promise->get_future();

        auto resultCb = [promise](uint16_t error_code, std::string error_msg, R result) {
            if (error_code != 0) {
                promise->set_exception(std::make_exception_ptr(std::runtime_error(std::move(error_msg))));
                return;
            }
            promise->set_value(std::move(result));
        };

        using ReturnFn = ipc::IPCCallback<R>;
        auto fullArgs = std::tuple_cat(std::move(args.args), std::make_tuple(ReturnFn(std::move(resultCb))));
        callServer(CallStruct<A..., ReturnFn>(args.methodId, std::move(fullArgs)));

        return future.get();
    }

    template <typename... Args>
    std::optional<uint16_t> subscribeToEventUpdates(uint16_t event_id, std::function<void(Args...)> onNotify)
    {
        if (!m_evClientId.has_value()) {
            m_evMemory->lock();
            m_evClientId = m_evMemory->read()->registerClient();
            m_evMemory->unlock();

            if (!m_evClientId.has_value())
                return std::nullopt;
        }

        std::unique_ptr<EventInfo> event_info = std::make_unique<EventInfo>();
        event_info->event_id = event_id;
        event_info->event_fn = [onNotify](const uint8_t *data, uint16_t) {
            if constexpr (sizeof...(Args) == 0) {
                onNotify();
                return;
            } else {
                auto argsTuple = exportFunctionArgTypes(onNotify);

                uint16_t offset = 0;
                fnPerTuple(
                    [&](auto &arg) -> int {
                        deserializer::deserializeTuple(arg, data, offset);
                        return 1;
                    },
                    argsTuple);

                std::apply(onNotify, argsTuple);
            }
        };

        std::unique_lock<std::mutex> lock(m_mtxEvData);
        m_evDataListeners.emplace_back(std::move(event_info));
        m_conditionEvData.notify_one();

        return m_evClientId;
    }

    void unsubscribeFromEventUpdates(uint16_t clientId);

private:
    template <typename C>
    std::shared_ptr<i_sm_object<C>> get_sm_object(const std::shared_ptr<i_sm_manager> &base)
    {
        auto typed = std::dynamic_pointer_cast<i_typed_sm_manager<C>>(base);
        return typed ? typed->getSharedMemory() : nullptr;
    }

    template <typename C>
    SharedMemoryPtr<C> connectToService(const std::string &name)
    {
        auto mem = i_sm_managers::create<C>(name);

        if (mem->isShared()) {
            std::cout << "[IClientIPC] Connected to existing service [" << mem->name() << "]" << std::endl;
            mem->loadFromShared();
        } else {
            static C space = C();
            mem->loadToShared(&space, sizeof(C));
            std::cout << "[IClientIPC] Connected to new service [" << mem->name() << "]" << std::endl;
        }

        return get_sm_object<C>(mem);
    }

    void pushCallWithNoArgs(uint16_t methodId, uint64_t callId, uint16_t cbIndex)
    {
        uint8_t serializedCall[MAX_CALL_SZ] = {'\0'};
        IPCCall callInfo {callId, methodId, m_clientId, cbIndex, 0u};
        const auto n = callInfo.serialize(serializedCall);
        m_callMemory->lock();
        m_callMemory->read()->push(serializedCall, n);
        m_callMemory->unlock();
    }

    template <typename... A>
    bool validateCall(const std::tuple<A...> &call_args, uint16_t& args_sz)
    {
        args_sz = 0;
        fnPerTuple(
            [&](const auto &arg) -> int {
                args_sz += serializer::sizeOfArgs(arg);
                return 1;
            },
            call_args);

        if (args_sz > MAX_CALL_SZ - IPCCall::HEAD_SZ) {
            return false;
        }

        return true;
    }

    template <typename... A>
    void pushCallWithArgs(const std::tuple<A...> &call_args, uint16_t method_id, size_t call_id, uint16_t cb_index)
    {
        uint8_t serialized_call[MAX_CALL_SZ] = {};

        IPCCall call_info {call_id, method_id, m_clientId, cb_index};
        uint32_t serialized_sz = call_info.serialize_with_args(serialized_call, call_args);

        m_callMemory->lock();
        m_callMemory->read()->push(serialized_call, serialized_sz);
        m_callMemory->unlock();
    }

    template <typename CallbackT, typename = std::enable_if_t<is_ipc_callback_v<std::decay_t<CallbackT>>>>
    std::optional<uint16_t> pushCb(uint16_t methodId, size_t callId, CallbackT &&cb, int timeout)
        requires is_ipc_callback_v<std::decay_t<CallbackT>>
    {
        using CallbackType = std::decay_t<CallbackT>;

        m_cbMemory->lock();
        const auto cbIndex = m_cbMemory->read()->pushCallback();
        m_cbMemory->unlock();

        if (!cbIndex.has_value()) {
            return cbIndex;
        }

        CallbackInfo cbInfo;
        cbInfo.timeout = std::chrono::high_resolution_clock().now() + std::chrono::milliseconds(timeout);
        cbInfo.cbIndex = cbIndex.value();
        cbInfo.fn = [this, methodId, callId, cb_ = std::move(cb), cbIndex_ = cbIndex.value(), timeout](uint8_t *data) {
            if (data == nullptr) {
                const std::string error_msg = std::format("IPC timeout ({} ms), methodId: {}, clientId: {}, callId: {}",
                                                          timeout,
                                                          methodId,
                                                          m_clientId,
                                                          callId);
                std::cerr << error_msg << std::endl;
                m_cbMemory->lock();
                m_cbMemory->read()->clearCallback(cbIndex_);
                m_cbMemory->unlock();
                cb_.fail(IPCError::Timeout, error_msg);
                return;
            }

            uint16_t error_code = 0;
            std::string error_msg;
            typename CallbackType::ArgsTuple args;
            CallbackType::deserialize(data, error_code, error_msg, args);

            if (error_code != IPCError::None) {
                cb_.fail(error_code, error_msg);
                return;
            }

            std::apply([&](auto &&...args_) { cb_.success(args_...); }, args);
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

    static constexpr uint16_t MAX_CALL_SZ = IPCConfig::User_CallSpace::CALL_SZ;
    static constexpr uint16_t MAX_EVENT_SZ = IPCConfig::User_EventSpace::EVENT_SZ;
    SharedMemoryPtr<IPCConfig::User_CallSpace> m_callMemory;
    SharedMemoryPtr<IPCConfig::User_CallbackSpace> m_cbMemory;
    SharedMemoryPtr<IPCConfig::User_EventSpace> m_evMemory;

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
        uint16_t cbIndex = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> timeout;
        Callback fn;
    };
    std::list<std::unique_ptr<CallbackInfo>> m_cbDataSubs;

    struct EventInfo final {
        using EventFn = std::function<void(uint8_t *, size_t)>;
        uint16_t event_id = 0;
        EventFn event_fn;
    };
    std::list<std::unique_ptr<EventInfo>> m_evDataListeners;
    std::optional<uint16_t> m_evClientId;

    template <uint16_t EventId, typename... Args>
    friend class IEventClientIPC;

    friend struct EventInfo;
};

} // namespace psi::ipc::client
