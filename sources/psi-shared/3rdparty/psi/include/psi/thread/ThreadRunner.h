#pragma once

#include <functional>

#include "CrashHandler.h"

#include "common/logger/Logger.h"

class ThreadRunner
{
public:
    using RunFn = std::function<void()>;
    using OnCrashFn = std::function<void()>;

    ThreadRunner(const std::string &threadName, RunFn &&runFn)
        : m_threadName(threadName)
        , m_runFn(std::forward<RunFn>(runFn))
    {
    }

    void run(OnCrashFn &&onCrashFn)
    {
        m_onCrashSub =
            m_crashHandler.crashEvent().subscribe([this, onCrashFn](const auto &errorMsg, const auto &stacktrace) {
                LOG_ERROR("Crash in " << m_threadName << " thread, [" << errorMsg << "]");
                LOG_ERROR(stacktrace);
                onCrashFn();
            });

        m_crashHandler.invoke([this]() {
            LOG_INFO("Run " << m_threadName);
            m_runFn();
        });

        m_onCrashSub.reset();
    }

private:
    const std::string m_threadName;
    psi::CrashHandler m_crashHandler;
    psi::Subscription m_onCrashSub;
    RunFn m_runFn;
};