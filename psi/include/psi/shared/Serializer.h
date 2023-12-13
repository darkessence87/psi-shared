#pragma once

#include <stdint.h>
#include <string>

namespace psi::ipc::serializer {

template <typename T>
inline size_t serializeType(uint8_t *buffer, const T &arg)
{
    const size_t sz = sizeof(T);
    memcpy(buffer, &arg, sz);
    return sz;
}

template <typename T, typename... Args>
inline size_t serializeType(uint8_t *buffer, const T &arg, Args &&...args)
{
    size_t r = serializeType<T>(buffer, arg);
    r += serializeType(buffer + r, std::forward<Args>(args)...);
    return r;
}

inline size_t serializeType(uint8_t *)
{
    return 0;
}

template <>
inline size_t serializeType<std::string>(uint8_t *buffer, const std::string &str)
{
    size_t len = str.size();
    size_t sz = serializeType(buffer, len);
    memcpy(&buffer[0] + sz, str.c_str(), len);
    return sz + len;
}

inline size_t serializeType(uint8_t *buffer, const std::string &str)
{
    size_t len = str.size();
    size_t sz = serializeType(buffer, len);
    memcpy(&buffer[0] + sz, str.c_str(), len);
    return sz + len;
}

template <typename T>
inline size_t sizeOfArgs(const T &)
{
    return sizeof(T);
}

template <typename T, typename... Args>
inline size_t sizeOfArgs(const T &arg, Args &&...args)
{
    size_t r = sizeOfArgs<T>(arg);
    r += sizeOfArgs(std::forward<Args>(args)...);
    return r;
}

template <>
inline size_t sizeOfArgs<std::string>(const std::string &arg)
{
    return sizeof(size_t) + arg.size();
}

inline size_t sizeOfArgs(const std::string &arg)
{
    return sizeof(size_t) + arg.size();
}

template <typename R>
inline void serializeTuple(uint8_t *buffer, const R &data, size_t &sz)
{
    sz += serializeType<R>(buffer + sz, data);
}

template <>
inline void serializeTuple<std::string>(uint8_t *buffer, const std::string &data, size_t &sz)
{
    serializeType<std::string>(buffer + sz, data);
    sz += sizeof(size_t) + data.size();
}

} // namespace psi::ipc::serializer
