#pragma once

namespace psi::ipc::client {

enum class CallStatus
{
    REMOTE_ERROR,
    SUCCESS
};

template <typename... A>
struct CallStruct {
    uint16_t methodId;
    std::tuple<A...> args;

    template <typename... U>
    explicit CallStruct(uint16_t id, U &&...u)
        : methodId(id)
        , args(std::forward<U>(u)...)
    {
    }
};

} // namespace psi::ipc::client
