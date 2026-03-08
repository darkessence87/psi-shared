#pragma once

#include <stdint.h>
#include <vector>

namespace psi::ipc {

using STRING_HEADER_TYPE = uint64_t;
static constexpr uint16_t STRING_HEADER_SIZE = sizeof(STRING_HEADER_TYPE);

using VECTOR_HEADER_TYPE = uint64_t;
static constexpr uint16_t VECTOR_HEADER_SIZE = sizeof(VECTOR_HEADER_TYPE);

using SIZE_TYPE = uint16_t;

template <typename T>
struct is_vector : std::false_type {
};

template <typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {
};

template <typename T>
concept VectorConcept = is_vector<T>::value;

template <typename T>
concept TrivialConcept = std::is_trivially_copyable_v<T>;

} // namespace psi::ipc
