
#pragma once

#include <sstream>

namespace psi::thread {

enum class LoopStrategy
{
    ASYNC_LOOP = 1,
    THREAD_POOL,
    THREAD_POOL_QUEUED,
};

inline std::ostream &operator<<(std::ostream &str, const LoopStrategy strat)
{
    switch (strat) {
    case LoopStrategy::ASYNC_LOOP:
        str << "ASYNC_LOOP";
        break;
    case LoopStrategy::THREAD_POOL:
        str << "THREAD_POOL";
        break;
    case LoopStrategy::THREAD_POOL_QUEUED:
        str << "THREAD_POOL_QUEUED";
        break;
    }
    return str;
}

} // namespace psi::thread