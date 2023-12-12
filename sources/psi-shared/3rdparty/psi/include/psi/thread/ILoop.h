#pragma once

#include <functional>

namespace psi::thread {

class ILoop
{
public:
    using Func = std::function<void()>;
    virtual ~ILoop() = default;

    virtual void run() = 0;
    virtual void invoke(Func &&) = 0;
    virtual void trigger() = 0;
    virtual void interrupt() = 0;
    virtual bool isRunning() = 0;
    virtual size_t getWorkload() const = 0;

    virtual void join() = 0;
};

} // namespace psi::thread
