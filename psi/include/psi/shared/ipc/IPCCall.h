
#pragma once

#include "protocol/Serializable.h"
#include "protocol/Serializer.h"
#include "psi/shared/ipc/TemplateHelpers.h"

namespace psi::ipc {

struct IPCCall {
    static constexpr uint16_t HEAD_SZ = 2 + 2 + 8 + 2;
    static constexpr uint16_t MAX_ARGS_SZ = 512u - HEAD_SZ - 2u;

    inline uint16_t serialize(uint8_t *buffer)
    {
        uint16_t offset = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        offset += serializable::write(buffer + offset, m_call_id);
        offset += serializable::write(buffer + offset, m_method_id);
        offset += serializable::write(buffer + offset, m_client_id);
        offset += serializable::write(buffer + offset, m_cb_index);
        offset += serializable::write(buffer + offset, m_args_sz);
#pragma clang diagnostic pop
        return offset;
    }

    template <typename... Args>
    inline uint32_t serialize_with_args(uint8_t *buffer, const std::tuple<Args...> &args)
    {
        uint32_t offset = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        offset += serializable::write(buffer + offset, m_call_id);
        offset += serializable::write(buffer + offset, m_method_id);
        offset += serializable::write(buffer + offset, m_client_id);
        offset += serializable::write(buffer + offset, m_cb_index);
        uint64_t args_sz = 0;
        fnPerTuple(
            [&](const auto &arg) -> int {
                serializer::serializeTuple(buffer + offset + sizeof(decltype(m_args_sz)), arg, args_sz);
                return 1;
            },
            args);
        m_args_sz = static_cast<uint16_t>(args_sz);
        serializable::write(buffer + offset, m_args_sz);
        offset += sizeof(decltype(m_args_sz)) + args_sz;
#pragma clang diagnostic pop
        return offset;
    }

    inline void deserialize(const uint8_t *const buffer, uint32_t offset)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        offset += serializable::read(buffer + offset, m_call_id);
        offset += serializable::read(buffer + offset, m_method_id);
        offset += serializable::read(buffer + offset, m_client_id);
        offset += serializable::read(buffer + offset, m_cb_index);
        offset += serializable::read(buffer + offset, m_args_sz);
#pragma clang diagnostic pop

        if (m_args_sz > MAX_ARGS_SZ) {
            /// @todo
        }
    }

    uint64_t m_call_id = 0u;

    uint16_t m_method_id = 0u;
    uint16_t m_client_id = 0u;
    uint16_t m_cb_index = 0u;
    uint16_t m_args_sz = 0u;
};

} // namespace psi::ipc
