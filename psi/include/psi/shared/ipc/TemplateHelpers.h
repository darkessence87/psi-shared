#pragma once

#include <tuple>

namespace psi {

template <typename F, typename Tuple>
constexpr void fnPerTuple(F &&fn, Tuple &&t)
{
    std::apply([&](auto &&...args) { (fn(std::forward<decltype(args)>(args)), ...); }, std::forward<Tuple>(t));
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

template <typename Tuple, size_t... I>
auto tuple_pop_back_impl(Tuple &&t, std::index_sequence<I...>)
{
    return std::make_tuple(std::get<I>(std::forward<Tuple>(t))...);
}

template <typename Tuple>
auto tuple_pop_back(Tuple &&t)
{
    constexpr size_t N = std::tuple_size_v<std::decay_t<Tuple>>;
    static_assert(N > 0, "tuple_pop_back requires non-empty tuple");
    return tuple_pop_back_impl(std::forward<Tuple>(t), std::make_index_sequence<N - 1> {});
}

} // namespace psi
