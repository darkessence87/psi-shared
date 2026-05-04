#pragma once

#include <any>
#include <functional>
#include <type_traits>

#include "psi/shared/ipc/TemplateHelpers.h"
#include "psi/shared/ipc/protocol/Deserializer.h"
#include "psi/shared/ipc/protocol/Serializer.h"

namespace psi::ipc {

template <typename T>
struct MemberFnWrapper {
    using OnCallbackResult = std::function<void(const uint8_t *, uint16_t)>;
    using Func = std::function<void(T &, const uint8_t *, OnCallbackResult)>;

    template <typename... A>
    MemberFnWrapper(void (T::*memberFn)(A...))
    {
        using ArgsTuple = std::tuple<A...>;
        constexpr size_t N = sizeof...(A);

        // case 1: no args, no callback = empty call
        if constexpr (N == 0) {
            m_any = Func([=](T &caller, auto...) mutable { (caller.*memberFn)(); });
        } else {
            using Last = std::tuple_element_t<N - 1, ArgsTuple>;
            constexpr bool has_cb = is_ipc_callback_v<Last>;

            if constexpr (!has_cb) {
                // case 2: args, no callback
                m_any = Func([=](T &caller, const uint8_t *args, auto...) mutable {
                    ArgsTuple params;
                    uint16_t offset = 0;

                    fnPerTuple(
                        [&](auto &arg) -> int {
                            deserializer::deserializeTuple(arg, args, offset);
                            return 1;
                        },
                        params);

                    std::apply([&](auto &&...unpacked) { (caller.*memberFn)(unpacked...); }, params);
                });
            } else {
                // case 3: no args, callback
                if constexpr (N == 1) {
                    m_any = Func([=](T &caller, auto, auto cb) mutable {
                        using CallbackT = std::decay_t<Last>;
                        CallbackT fn {[cb](uint16_t error_code, const std::string &error_msg, auto... args_) {
                            constexpr size_t DATA_SIZE = psi::ipc::IPCConfig::User_CallbackSpace::DATA_SIZE;
                            uint8_t data[DATA_SIZE] = {};
                            const auto n = CallbackT::serialize(data, error_code, error_msg, args_...);
                            cb(data, n);
                        }};
                        (caller.*memberFn)(fn);
                    });
                } else {
                    // case 4: args, callback
                    m_any = Func([=](T &caller, const uint8_t *args, auto cb) mutable {
                        using ArgTupleNoCb = decltype(tuple_pop_back(std::declval<ArgsTuple>()));
                        ArgTupleNoCb params;
                        uint16_t offset = 0;

                        fnPerTuple(
                            [&](auto &arg) -> int {
                                deserializer::deserializeTuple(arg, args, offset);
                                return 1;
                            },
                            params);
                        using CallbackT = std::decay_t<Last>;
                        CallbackT fn {[cb](uint16_t error_code, const std::string &error_msg, auto... args_) {
                            constexpr size_t DATA_SIZE = psi::ipc::IPCConfig::User_CallbackSpace::DATA_SIZE;
                            uint8_t data[DATA_SIZE] = {};
                            const auto n = CallbackT::serialize(data, error_code, error_msg, args_...);
                            cb(data, n);
                        }};
                        std::apply([&](auto &&...unpacked) { (caller.*memberFn)(unpacked..., fn); }, params);
                    });
                }
            }
        }
    }

    template <typename R, typename... A>
    MemberFnWrapper(R (T::*memberFn)(A...))
    {
        auto callArgs = std::apply(
            [](auto... ts) {
                return std::tuple_cat(std::conditional_t<(std::is_trivially_copyable<decltype(ts)>::value
                                                          || std::is_same<std::string, decltype(ts)>::value),
                                                         std::tuple<decltype(ts)>,
                                                         std::tuple<>> {}...);
            },
            std::tuple<A...>());
        auto cbType = std::apply(
            [](auto... ts) {
                return std::tuple_cat(std::conditional_t<(!std::is_trivially_copyable<decltype(ts)>::value
                                                          && !std::is_same<std::string, decltype(ts)>::value),
                                                         std::tuple<decltype(ts)>,
                                                         std::tuple<>> {}...);
            },
            std::tuple<A...>());

        // case 1: no args = empty call
        if constexpr (std::tuple_size_v<decltype(callArgs)> == 0 && std::tuple_size_v<decltype(cbType)> == 0) {
            m_any = Func([=](T &caller, auto, auto cb) mutable {
                R r = std::apply(memberFn, std::tuple<T &>(caller));

                using CallbackT = IPCCallback<R>;
                CallbackT fn {[cb](uint16_t error_code, const std::string &error_msg, auto... args_) {
                    constexpr size_t DATA_SIZE = psi::ipc::IPCConfig::User_CallbackSpace::DATA_SIZE;
                    uint8_t data[DATA_SIZE] = {};
                    const auto n = CallbackT::serialize(data, error_code, error_msg, args_...);
                    cb(data, n);
                }};
                fn.success(r);
            });
        }
        // case 2: args
        else if constexpr (std::tuple_size_v<decltype(cbType)> == 0) {
            m_any = Func([=](T &caller, const uint8_t *args, auto cb) mutable {
                uint16_t offset = 0;
                fnPerTuple(
                    [&](auto &result) -> int {
                        deserializer::deserializeTuple(result, args, offset);
                        return 1;
                    },
                    callArgs);
                auto params = std::tuple_cat(std::tuple<T &>(caller), callArgs);
                R r = std::apply(memberFn, params);

                using CallbackT = IPCCallback<R>;
                CallbackT fn {[cb](uint16_t error_code, const std::string &error_msg, auto... args_) {
                    constexpr size_t DATA_SIZE = psi::ipc::IPCConfig::User_CallbackSpace::DATA_SIZE;
                    uint8_t data[DATA_SIZE] = {};
                    const auto n = CallbackT::serialize(data, error_code, error_msg, args_...);
                    cb(data, n);
                }};
                fn.success(r);
            });
        }
    }

    void operator()(T &caller, const uint8_t *args, OnCallbackResult cb) const
    {
        std::invoke(std::any_cast<Func>(m_any), caller, args, cb);
    }

    std::any m_any;
};

} // namespace psi::ipc
