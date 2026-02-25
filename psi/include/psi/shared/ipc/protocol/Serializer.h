#pragma once

#include <stdint.h>
#include <string>

namespace psi::ipc::serializer {

template <typename T>
inline uint16_t serializeType(uint8_t *buffer, const T &arg)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    const uint16_t sz = sizeof(T);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    std::memcpy(buffer, &arg, sz);
#pragma clang diagnostic pop
    return sz;
}

template <typename T, typename... Args>
inline uint16_t serializeType(uint8_t *buffer, const T &arg, Args &&...args)
{
    uint16_t sz = serializeType<T>(buffer, arg);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    sz += serializeType(buffer + sz, std::forward<Args>(args)...);
#pragma clang diagnostic pop
    return sz;
}

inline uint16_t serializeType(uint8_t *)
{
    return 0;
}

template <>
inline uint16_t serializeType<std::string>(uint8_t *buffer, const std::string &str)
{
    const uint16_t len = static_cast<uint16_t>(str.size());
    const uint16_t sz = serializeType(buffer, uint64_t(len));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    std::memcpy(buffer + sz, str.data(), len);
#pragma clang diagnostic pop
    return sz + len;
}

template <typename T>
inline uint16_t sizeOfArgs(const T &)
{
    return sizeof(T);
}

template <typename T, typename... Args>
inline uint16_t sizeOfArgs(const T &arg, Args &&...args)
{
    uint16_t r = sizeOfArgs<T>(arg);
    r += sizeOfArgs(std::forward<Args>(args)...);
    return r;
}

template <>
inline uint16_t sizeOfArgs<std::string>(const std::string &arg)
{
    return sizeof(uint64_t) + static_cast<uint16_t>(arg.size());
}

inline uint16_t sizeOfArgs(const std::string &arg)
{
    return sizeof(uint64_t) + static_cast<uint16_t>(arg.size());
}

template <typename R>
inline void serializeTuple(uint8_t *buffer, const R &data, uint64_t &sz)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    sz += serializeType<R>(buffer + sz, data);
#pragma clang diagnostic pop
}

template <>
inline void serializeTuple<std::string>(uint8_t *buffer, const std::string &data, uint64_t &sz)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    sz += serializeType<std::string>(buffer + sz, data);
#pragma clang diagnostic pop
}

} // namespace psi::ipc::serializer
