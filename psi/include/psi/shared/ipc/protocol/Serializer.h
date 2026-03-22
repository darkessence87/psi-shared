#pragma once

#include "Common.h"

#include <string>

namespace psi::ipc::serializer {

template <typename T>
inline SIZE_TYPE serializeType(uint8_t *buffer, const T &arg);

template <TrivialConcept T>
inline SIZE_TYPE serializeType(uint8_t *buffer, const T &arg)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    const SIZE_TYPE sz = sizeof(T);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
    std::memcpy(buffer, &arg, sz);
#pragma clang diagnostic pop
    return sz;
}

template <typename T, typename... Args>
inline SIZE_TYPE serializeType(uint8_t *buffer, const T &arg, Args &&...args)
{
    SIZE_TYPE sz = serializeType<T>(buffer, arg);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    sz += serializeType(buffer + sz, std::forward<Args>(args)...);
#pragma clang diagnostic pop
    return sz;
}

inline SIZE_TYPE serializeType(uint8_t *)
{
    return 0;
}

template <>
inline SIZE_TYPE serializeType<std::string>(uint8_t *buffer, const std::string &str)
{
    const auto len = static_cast<SIZE_TYPE>(str.size());
    const SIZE_TYPE sz = serializeType(buffer, STRING_HEADER_TYPE(len));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    std::memcpy(buffer + sz, str.data(), len);
#pragma clang diagnostic pop
    return sz + len;
}

template <VectorConcept C>
inline SIZE_TYPE serializeType(uint8_t *buffer, const C &data)
{
    SIZE_TYPE offset = 0;
    const SIZE_TYPE sz = serializeType(buffer, VECTOR_HEADER_TYPE(data.size()));
    offset += sz;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    for (const auto& data_element : data) {
        offset += serializeType(buffer + offset, data_element);
    }
#pragma clang diagnostic pop
    return offset;
}

template <typename T>
inline SIZE_TYPE sizeOfArgs(const T &)
{
    return sizeof(T);
}

template <typename T, typename... Args>
inline SIZE_TYPE sizeOfArgs(const T &arg, Args &&...args)
{
    SIZE_TYPE r = sizeOfArgs<T>(arg);
    r += sizeOfArgs(std::forward<Args>(args)...);
    return r;
}

template <>
inline SIZE_TYPE sizeOfArgs<std::string>(const std::string &arg)
{
    return STRING_HEADER_SIZE + static_cast<SIZE_TYPE>(arg.size());
}

inline SIZE_TYPE sizeOfArgs(const std::string &arg)
{
    return STRING_HEADER_SIZE + static_cast<SIZE_TYPE>(arg.size());
}

template <typename R>
inline void serializeTuple(uint8_t *buffer, const R &data, SIZE_TYPE &sz)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    sz += serializeType<R>(buffer + sz, data);
#pragma clang diagnostic pop
}

template <>
inline void serializeTuple<std::string>(uint8_t *buffer, const std::string &data, SIZE_TYPE &sz)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    sz += serializeType<std::string>(buffer + sz, data);
#pragma clang diagnostic pop
}

} // namespace psi::ipc::serializer
