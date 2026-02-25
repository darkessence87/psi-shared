
#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace psi::ipc::serializable {

template <typename T>
using base_t = std::remove_cvref_t<T>;

template <typename T>
concept fixed_width =
    std::same_as<base_t<T>, uint8_t> || std::same_as<base_t<T>, uint16_t> || std::same_as<base_t<T>, uint32_t>
    || std::same_as<base_t<T>, uint64_t> || std::same_as<base_t<T>, int8_t> || std::same_as<base_t<T>, int16_t>
    || std::same_as<base_t<T>, int32_t> || std::same_as<base_t<T>, int64_t>;

template <typename T>
concept serializable_type =
    fixed_width<T> || (std::is_trivially_copyable_v<base_t<T>> && std::is_standard_layout_v<base_t<T>>);

template <serializable_type T>
inline size_t write(uint8_t *buffer, const T &data)
{
    constexpr size_t sz = sizeof(T);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    std::memcpy(buffer, &data, sz);
#pragma clang diagnostic pop
    return sz;
}

template <serializable_type T>
inline size_t read(const uint8_t *const buffer, T &data)
{
    constexpr size_t sz = sizeof(T);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    std::memcpy(&data, buffer, sz);
#pragma clang diagnostic pop
    return sz;
}

} // namespace psi::ipc::serializable
