#pragma once

#include <stdint.h>
#include <string.h>
#include <string>
#include <tuple>

namespace psi::ipc::deserializer {

template <typename R>
inline R deserializeType(const uint8_t *data, uint64_t offset = 0)
{
    static_assert(std::is_trivially_copyable_v<R>, "R must be trivially copyable");

    R result;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    std::memcpy(&result, data + offset, sizeof(R));
#pragma clang diagnostic pop
    return result;
}

template <>
inline std::string deserializeType<std::string>(const uint8_t *const data, uint64_t offset)
{
    const uint64_t len = deserializeType<uint64_t>(data, offset);
    std::string result;
    result.resize(len);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    std::memcpy(result.data(), data + offset + sizeof(uint64_t), len);
#pragma clang diagnostic pop
    return result;
}

template <typename R>
inline void deserializeTuple(R &result, const uint8_t *const data, uint64_t &offset)
{
    result = deserializeType<R>(data, offset);
    offset += sizeof(R);
}

template <>
inline void deserializeTuple<std::string>(std::string &result, const uint8_t *const data, uint64_t &offset)
{
    result = deserializeType<std::string>(data, offset);
    offset += sizeof(uint64_t) + result.size();
}

template <typename R>
inline std::tuple<R> deserializeTypeVariadic(const uint8_t *const data, uint64_t offset, R * = nullptr)
{
    return {deserializeType<R>(data, offset)};
}

template <typename T, typename... R>
inline std::tuple<T, R...> deserializeTypeVariadic(const uint8_t *const data, uint64_t offset, T *, R *...)
{
    T head;
    uint64_t local_offset = offset;
    deserializeTuple(head, data, local_offset);
    auto tail = deserializeTypeVariadic<R...>(data, local_offset, static_cast<R *>(nullptr)...);
    return std::tuple_cat(std::make_tuple(std::move(head)), tail);
}

} // namespace psi::ipc::deserializer
