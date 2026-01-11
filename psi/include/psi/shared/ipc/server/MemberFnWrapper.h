#pragma once

#include <any>
#include <functional>
#include <type_traits>

#include "psi/shared/Deserializer.h"
#include "psi/shared/Serializer.h"

namespace psi::ipc {

template <typename F, typename... T, size_t... Index>
constexpr auto fnPerTupleImpl(F &&fn, std::tuple<T...> &t, std::index_sequence<Index...>)
{
#ifdef __linux__
    __attribute__((__unused__)) auto x = {fn(std::get<Index>(t))...};
#else
    [[maybe_unused]] auto x = {fn(std::get<Index>(t))...};
#endif
}

template <int Trim, typename... T, typename F>
constexpr auto fnPerTuple(F &&fn, std::tuple<T...> &t)
{
    return fnPerTupleImpl(fn, t, std::make_index_sequence<sizeof...(T) - Trim>());
}

template <typename T>
struct MemberFnWrapper {
    using OnCallbackResult = std::function<void(const uint8_t *, size_t)>;
    using Func = std::function<void(T &, const uint8_t *, size_t, OnCallbackResult)>;

    template <typename... A>
    MemberFnWrapper(void (T::*memberFn)(A...))
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

        // case 1: no args, no callback = empty call
        if constexpr (std::tuple_size_v<decltype(callArgs)> == 0 && std::tuple_size_v<decltype(cbType)> == 0) {
            m_any = Func([=](T &caller, auto...) mutable { std::apply(memberFn, std::tuple<T &>(caller)); });
        }
        // case 2: args, no callback
        else if constexpr (std::tuple_size_v<decltype(cbType)> == 0) {
            m_any = Func([=](T &caller, const uint8_t *args, size_t offset, auto...) mutable {
                fnPerTuple<0u>(
                    [&](auto &result) -> int {
                        deserializer::deserializeTuple(result, args, offset);
                        return 1;
                    },
                    callArgs);
                auto params = std::tuple_cat(std::tuple<T &>(caller), callArgs);
                std::apply(memberFn, params);
            });
        }
        // case 3: no args, callback
        else if constexpr (std::tuple_size_v<decltype(callArgs)> == 0) {
            m_any = Func([=](T &caller, auto, auto, auto cb) mutable {
                std::get<0>(cbType) = [cb](auto... args) {
                    uint8_t data[512] = {};
                    const auto n = serializer::serializeType(data, args...);
                    cb(data, n);
                };
                auto params = std::tuple_cat(std::tuple<T &>(caller), cbType);
                std::apply(memberFn, params);
            });
        }
        // case 4: args, callback
        else {
            m_any = Func([=](T &caller, const uint8_t *args, size_t offset, auto cb) mutable {
                fnPerTuple<0u>(
                    [&](auto &result) -> int {
                        deserializer::deserializeTuple(result, args, offset);
                        return 1;
                    },
                    callArgs);
                std::get<0>(cbType) = [cb](auto... args) {
                    uint8_t data[512] = {};
                    const auto n = serializer::serializeType(data, args...);
                    cb(data, n);
                };
                auto params = std::tuple_cat(std::tuple<T &>(caller), callArgs, cbType);
                std::apply(memberFn, params);
            });
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
            m_any = Func([=](T &caller, auto, auto, auto cb) mutable {
                R r = std::apply(memberFn, std::tuple<T &>(caller));
                uint8_t data[512] = {};
                const auto n = serializer::serializeType(data, r);
                cb(data, n);
            });
        }
        // case 2: args
        else if constexpr (std::tuple_size_v<decltype(cbType)> == 0) {
            m_any = Func([=](T &caller, const uint8_t *args, size_t offset, auto cb) mutable {
                fnPerTuple<0u>(
                    [&](auto &result) -> int {
                        deserializer::deserializeTuple(result, args, offset);
                        return 1;
                    },
                    callArgs);
                auto params = std::tuple_cat(std::tuple<T &>(caller), callArgs);
                R r = std::apply(memberFn, params);
                uint8_t data[512] = {};
                const auto n = serializer::serializeType(data, r);
                cb(data, n);
            });
        }
    }

    void operator()(T &caller, const uint8_t *args, size_t offset, OnCallbackResult cb) const
    {
        std::invoke(std::any_cast<Func>(m_any), caller, args, offset, cb);
    }

    std::any m_any;
};

} // namespace psi::ipc
