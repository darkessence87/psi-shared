#pragma once

#include "psi/shared/Deserializer.h"
#include "psi/shared/Serializer.h"

namespace psi::ipc {

#define PACKED
#pragma pack(push, 1)
struct IPCCall {
    IPCCall()
        : callId(0)
        , cbIndex(0)
        , methodId(0)
        , clientId(0)
    {
    }

    IPCCall(uint16_t method, uint16_t client, size_t call, size_t cbIndex)
        : callId(call)
        , cbIndex(cbIndex)
        , methodId(method)
        , clientId(client)
    {
    }

    size_t callId;
    size_t cbIndex;
    uint16_t methodId;
    uint16_t clientId;
} PACKED;
#pragma pack(pop)
#undef PACKED

template <>
inline size_t serializer::serializeType(uint8_t *buffer, const ipc::IPCCall &arg)
{
    memcpy(buffer, &arg.methodId, 2);
    memcpy(buffer + 2, &arg.clientId, 2);
    memcpy(buffer + 4, &arg.callId, sizeof(size_t));
    memcpy(buffer + 4 + sizeof(size_t), &arg.cbIndex, sizeof(size_t));
    return 4 + 2 * sizeof(size_t);
}

template <>
inline ipc::IPCCall deserializer::deserializeType(const uint8_t *const data, size_t offset)
{
    ipc::IPCCall result = ipc::IPCCall();
    memcpy(&result.methodId, &data[0] + offset, 2);
    memcpy(&result.clientId, &data[0] + offset + 2, 2);
    memcpy(&result.callId, &data[0] + offset + 4, sizeof(size_t));
    memcpy(&result.cbIndex, &data[0] + offset + 4 + sizeof(size_t), sizeof(size_t));
    return result;
}

} // namespace psi::ipc
