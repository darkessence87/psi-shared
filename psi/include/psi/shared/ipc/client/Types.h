#pragma once

namespace psi::ipc::client {

enum class CallStatus
{
    REMOTE_ERROR,
    SUCCESS
};

template <typename... A>
struct CallStruct {
    CallStruct(uint16_t methodId_, A... args_)
        : methodId(methodId_)
        , args(args_...)
    {
    }

    CallStruct(uint16_t methodId_, std::tuple<A...> args_)
        : methodId(methodId_)
        , args(args_)
    {
    }

    uint16_t methodId = 0;
    std::tuple<A...> args;
};

} // namespace psi::ipc::client
