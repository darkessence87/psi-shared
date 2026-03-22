#pragma once

#include "Common.h"

#include <string>
#include <tuple>

namespace psi::ipc::deserializer {

template <typename R>
inline R deserializeType(const uint8_t *data, SIZE_TYPE& offset);

template <TrivialConcept R>
inline R deserializeType(const uint8_t *data, SIZE_TYPE& offset)
{
    static_assert(std::is_trivially_copyable_v<R>, "R must be trivially copyable");

    R result;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    std::memcpy(&result, data + offset, sizeof(R));
#pragma clang diagnostic pop
    offset += sizeof(R);
    return result;
}

template <>
inline std::string deserializeType<std::string>(const uint8_t *data, SIZE_TYPE& offset)
{
    const uint64_t len = deserializeType<STRING_HEADER_TYPE>(data, offset);
    std::string result;
    result.resize(len);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    std::memcpy(result.data(), data + offset, len);
#pragma clang diagnostic pop
    offset += len;
    return result;
}

template <VectorConcept C>
inline C deserializeType(const uint8_t *const data, SIZE_TYPE& offset)
{
    using T = typename C::value_type;

    const uint64_t vector_sz = deserializeType<VECTOR_HEADER_TYPE>(data, offset);

    std::vector<T> result;
    result.resize(vector_sz);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    for (uint64_t element_index = 0; element_index < vector_sz; ++element_index) {
        result[element_index] = deserializeType<T>(data, offset);
    }
#pragma clang diagnostic pop
    return result;
}

template <typename R>
inline void deserializeTuple(R &result, const uint8_t *const data, SIZE_TYPE &offset)
{
    result = deserializeType<R>(data, offset);
}

template <>
inline void deserializeTuple<std::string>(std::string &result, const uint8_t *const data, SIZE_TYPE &offset)
{
    result = deserializeType<std::string>(data, offset);
}

template <VectorConcept C>
inline void deserializeTuple(C &result, const uint8_t *const data, SIZE_TYPE &offset)
{
    result = deserializeType<C>(data, offset);
}

template <typename R>
inline std::tuple<R> deserializeTypeVariadic(const uint8_t *const data, SIZE_TYPE offset, R * = nullptr)
{
    return {deserializeType<R>(data, offset)};
}

template <typename T, typename... R>
inline std::tuple<T, R...> deserializeTypeVariadic(const uint8_t *const data, SIZE_TYPE offset, T *, R *...)
{
    T head;
    SIZE_TYPE local_offset = offset;
    deserializeTuple(head, data, local_offset);
    auto tail = deserializeTypeVariadic<R...>(data, local_offset, static_cast<R *>(nullptr)...);
    return std::tuple_cat(std::make_tuple(std::move(head)), tail);
}

} // namespace psi::ipc::deserializer
