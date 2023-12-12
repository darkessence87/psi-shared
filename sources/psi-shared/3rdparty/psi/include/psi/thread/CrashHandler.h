#pragma once

#include <functional>
#include <string>

#include "psi/comm/Event.h"

namespace psi::thread {

class CrashHandler final
{
    using CrashEvent = comm::Event<std::string /*error*/, std::string /*stacktrace*/>;

public:
    using Func = std::function<void()>;

    CrashHandler();

    void invoke(Func &&);
    CrashEvent::Interface &crashEvent();

private:
    static void handleSignals();
    void handleException(std::string & /*stacktrace*/);
    void invokeImpl(Func &&);

private:
    CrashEvent m_crashEvent;
};

} // namespace psi::thread