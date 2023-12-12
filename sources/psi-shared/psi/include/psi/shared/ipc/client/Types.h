#pragma once

namespace psi::ipc::client {

enum class CallStatus
{
    REMOTE_ERROR,
    SUCCESS
};

template <typename... A>
struct CallStruct {
    CallStruct(uint16_t methodId, A... args)
        : methodId(methodId)
        , args(args...)
    {
    }

    CallStruct(uint16_t methodId, std::tuple<A...> args)
        : methodId(methodId)
        , args(args)
    {
    }

    uint16_t methodId = 0;
    std::tuple<A...> args;
};

} // namespace psi::ipc::client