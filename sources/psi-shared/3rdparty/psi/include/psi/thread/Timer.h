
#pragma once

#include <atomic>
#include <functional>
#include <memory>

namespace psi::thread {

class TimerLoop;

class Timer final : public std::enable_shared_from_this<Timer>
{
    using Func = std::function<void()>;

public:
    Timer(size_t, TimerLoop &);
    ~Timer();

    void start(int /*milliseconds*/, const Func &);
    void restart();
    void stop();
    bool isRunning() const;

private:
    void invoke();

private:
    TimerLoop &m_loop;
    const size_t m_timerId;
    Func m_function;
    int m_length;

    std::atomic<bool> m_isActive;

    friend class TimerLoop;

private:
    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;
};

} // namespace psi::thread
