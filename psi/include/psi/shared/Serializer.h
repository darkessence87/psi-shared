#pragma once

#include <stdint.h>
#include <string>

#include "psi/tools/Tools.h"

namespace psi::ipc::serializer {

template <typename T>
inline uint64_t serializeType(uint8_t *buffer, const T &arg)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    const uint64_t sz = sizeof(T);
    tools::mem_copy(buffer, 0, &arg, 0, sz);
    return sz;
}

template <typename T, typename... Args>
inline uint64_t serializeType(uint8_t *buffer, const T &arg, Args &&...args)
{
    uint64_t sz = serializeType<T>(buffer, arg);
    sz += serializeType(tools::shift_ptr(buffer, sz), std::forward<Args>(args)...);
    return sz;
}

inline uint64_t serializeType(uint8_t *)
{
    return 0;
}

template <>
inline uint64_t serializeType<std::string>(uint8_t *buffer, const std::string &str)
{
    const uint64_t len = str.size();
    const uint64_t sz = serializeType(buffer, len);
    tools::mem_copy(buffer, sz, str.data(), 0, len);
    return sz + len;
}

template <typename T>
inline uint64_t sizeOfArgs(const T &)
{
    return sizeof(T);
}

template <typename T, typename... Args>
inline uint64_t sizeOfArgs(const T &arg, Args &&...args)
{
    uint64_t r = sizeOfArgs<T>(arg);
    r += sizeOfArgs(std::forward<Args>(args)...);
    return r;
}

template <>
inline uint64_t sizeOfArgs<std::string>(const std::string &arg)
{
    return sizeof(uint64_t) + arg.size();
}

inline uint64_t sizeOfArgs(const std::string &arg)
{
    return sizeof(uint64_t) + arg.size();
}

template <typename R>
inline void serializeTuple(uint8_t *buffer, const R &data, uint64_t &sz)
{
    sz += serializeType<R>(tools::shift_ptr(buffer, sz), data);
}

template <>
inline void serializeTuple<std::string>(uint8_t *buffer, const std::string &data, uint64_t &sz)
{
    sz += serializeType<std::string>(tools::shift_ptr(buffer, sz), data);
}

} // namespace psi::ipc::serializer
