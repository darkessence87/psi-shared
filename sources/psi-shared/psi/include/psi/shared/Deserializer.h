#pragma once

#include <stdint.h>
#include <string.h>
#include <string>
#include <tuple>

namespace psi::ipc::deserializer {

template <typename R>
inline R deserializeType(const uint8_t *const data, size_t offset = 0)
{
    R result = R();
    const size_t sz = sizeof(R);
    memcpy(&result, &data[0] + offset, sz);
    return result;
}

template <>
inline std::string deserializeType<std::string>(const uint8_t *const data, size_t offset)
{
    const size_t len = deserializeType<size_t>(data, offset);
    std::string result;
    result.resize(len);
    memcpy(&result[0], &data[0] + offset + sizeof(size_t), len);
    return result;
}

template <typename R>
inline std::tuple<R> deserializeTypeVariadic(const uint8_t *const data, size_t offset, R * = nullptr)
{
    return {deserializeType<R>(data, offset)};
}

template <typename T, typename... R>
inline std::tuple<T, R...> deserializeTypeVariadic(const uint8_t *const data, size_t offset, T *, R *...)
{
    auto t = deserializeTypeVariadic<T>(data, offset);
    auto values = deserializeTypeVariadic<R...>(data, offset + sizeof(T), static_cast<R *>(nullptr)...);
    return std::tuple_cat(t, values);
}

template <typename R>
inline void deserializeTuple(R &result, const uint8_t *const data, size_t &offset)
{
    result = deserializeType<R>(data, offset);
    offset += sizeof(R);
}

template <>
inline void deserializeTuple<std::string>(std::string &result, const uint8_t *const data, size_t &offset)
{
    result = deserializeType<std::string>(data, offset);
    offset += sizeof(size_t) + result.size();
}

} // namespace psi::ipc::deserializer
