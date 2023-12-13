#pragma once

#include <tuple>

namespace psi {

template <typename F, typename... T, size_t... Index>
constexpr auto fnPerTupleImpl(F &&fn, std::tuple<T...> &t, std::index_sequence<Index...>)
{
#ifdef __linux__
    __attribute__((__unused__)) auto x = {fn(std::get<Index>(t))...};
#else
    auto x = {fn(std::get<Index>(t))...};
#endif
}

template <typename F, typename... T, size_t... Index>
constexpr auto fnPerTupleImpl(F &&fn, const std::tuple<T...> &t, std::index_sequence<Index...>)
{
#ifdef __linux__
    __attribute__((__unused__)) auto x = {fn(std::get<Index>(t))...};
#else
    auto x = {fn(std::get<Index>(t))...};
#endif
}

template <int Trim, typename... T, typename F>
constexpr auto fnPerTuple(F &&fn, std::tuple<T...> &t)
{
    return fnPerTupleImpl(fn, t, std::make_index_sequence<sizeof...(T) - Trim>());
}

template <int Trim, typename... T, typename F>
constexpr auto fnPerTuple(F &&fn, const std::tuple<T...> &t)
{
    return fnPerTupleImpl(fn, t, std::make_index_sequence<sizeof...(T) - Trim>());
}

template <std::size_t... I, typename T1, typename T2>
void copy_tuple_impl(T1 const &from, T2 &to, std::index_sequence<I...>)
{
    int dummy[] = {(std::get<I>(to) = std::get<I>(from), 0)...};
    static_cast<void>(dummy);
}

template <typename T1, typename T2>
void copy_tuple(T1 const &from, T2 &to)
{
    if constexpr (std::tuple_size<T1>::value != 0 && std::tuple_size<T2>::value != 0) {
        if constexpr (std::tuple_size<T1>::value >= std::tuple_size<T2>::value) {
            copy_tuple_impl(from, to, std::make_index_sequence<std::tuple_size<T2>::value>());
        } else {
            copy_tuple_impl(from, to, std::make_index_sequence<std::tuple_size<T1>::value>());
        }
    }
}

template <typename... A>
std::tuple<A...> exportFunctionArgTypes(std::function<void(A...)>)
{
    return std::tuple<A...>();
}

} // namespace psi