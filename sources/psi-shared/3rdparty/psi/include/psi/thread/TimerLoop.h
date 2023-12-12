#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "psi/comm/Subscription.h"
#include "psi/thread/Timer.h"

namespace psi::thread {

class TimerLoop
{
    using Func = std::function<void()>;
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

public:
    TimerLoop();
    virtual ~TimerLoop();

    void addTimer(std::shared_ptr<Timer>, int);
    void restartTimer(size_t);
    void removeTimer(size_t);

    void trigger();
    void interrupt();
    bool isRunning();

private:
    void onThreadUpdate();

private:
    TimePoint m_tempExecutionTime;
    TimePoint m_nextExecutionTime;
    std::map<TimePoint, std::vector<std::shared_ptr<Timer>>> m_queue;
    std::map<size_t, TimePoint> m_timersPlan;

    bool m_isActive;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_thread;

    psi::comm::Subscription m_crashSub;
};

} // namespace psi::thread