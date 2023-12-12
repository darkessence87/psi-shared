#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <vector>

#include "psi/comm/Subscription.h"

namespace psi::thread {

class PostponeLoop
{
    using Func = std::function<void()>;
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

public:
    PostponeLoop();
    virtual ~PostponeLoop();

    void invoke(Func &&, const TimePoint &);
    void trigger();
    void interrupt();
    bool isRunning();

private:
    void onThreadUpdate();

private:
    TimePoint m_nextExecutionTime;
    std::map<TimePoint, std::vector<Func>> m_queue;

    bool m_isActive;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_thread;

    psi::comm::Subscription m_crashSub;
};

} // namespace psi::thread