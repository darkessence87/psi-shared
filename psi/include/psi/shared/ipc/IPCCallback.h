
#pragma once

#include <cstring>
#include <functional>

#include "IPCError.h"

#include "psi/shared/ipc/TemplateHelpers.h"
#include "psi/shared/ipc/protocol/Deserializer.h"
#include "psi/shared/ipc/protocol/Serializer.h"

namespace psi::ipc {

template <typename... Args>
class IPCCallback final
{
public:
    using ArgsTuple = std::tuple<Args...>;
    using OnCallbackFn = std::function<void(uint16_t /*error_code*/, std::string /*error_msg*/, Args...)>;

    template <typename F>
        requires(!std::is_same_v<std::decay_t<F>, IPCCallback>)
    IPCCallback(F &&fn)
        : m_cb(std::forward<F>(fn))
    {
    }

    void success(Args... args) const
    {
        if (m_cb) {
            m_cb(IPCError::None, std::string {}, args...);
        }
    }

    void fail(uint16_t error_code, const std::string &error_msg = std::string {}) const
    {
        if (m_cb) {
            m_cb(error_code, error_msg, Args {}...);
        }
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    static inline uint16_t serialize(uint8_t *buffer, uint16_t error_code, const std::string &error_msg, const Args &...args)
    {
        uint16_t offset = 0;

        std::memcpy(buffer + offset, &error_code, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        if (error_code != IPCError::None) {
            offset += serializer::serializeType(buffer + offset, error_msg);
            return offset;
        }

        uint16_t payload_sz = 0;
        uint16_t payload_offset = offset + sizeof(uint16_t);
        if constexpr (sizeof...(Args) > 0) {
            uint16_t inner_sz = 0;
            fnPerTuple(
                [&](const auto &arg) -> int {
                    serializer::serializeTuple(buffer + payload_offset, arg, inner_sz);
                    return 1;
                },
                std::tuple<Args...>(args...));
            payload_sz = static_cast<uint16_t>(inner_sz);
        }
        std::memcpy(buffer + offset, &payload_sz, sizeof(uint16_t));
        offset += sizeof(uint16_t) + payload_sz;

        return offset;
    }

    static inline void deserialize(uint8_t *data, uint16_t &error_code, std::string &error_msg, ArgsTuple &args)
    {
        uint16_t offset = 0;

        error_code = deserializer::deserializeType<uint16_t>(data, offset);
        if (error_code != IPCError::None) {
            error_msg = deserializer::deserializeType<std::string>(data, offset);
            return;
        }

        uint16_t payload_sz = deserializer::deserializeType<uint16_t>(data, offset);
        if constexpr (sizeof...(Args) == 0) {
            return;
        }

        uint16_t innerOffset = 0;
        fnPerTuple(
            [&](auto &arg) -> int {
                deserializer::deserializeTuple(arg, data + offset, innerOffset);
                return 1;
            },
            args);
        offset += innerOffset;
    }
#pragma clang diagnostic pop

private:
    OnCallbackFn m_cb = nullptr;
};

template <typename T>
struct is_ipc_callback : std::false_type {
};

template <typename... Args>
struct is_ipc_callback<IPCCallback<Args...>> : std::true_type {
};

template <typename T>
inline constexpr bool is_ipc_callback_v = is_ipc_callback<std::decay_t<T>>::value;

} // namespace psi::ipc
