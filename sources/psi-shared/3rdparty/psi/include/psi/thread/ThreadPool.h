#pragma once

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "ILoop.h"
#include "psi/comm/Subscription.h"

namespace psi::thread {

class ThreadPool : public ILoop
{
public:
    ThreadPool(uint8_t numberOfThreads);
    virtual ~ThreadPool();

public: /// implements ILoop
    void run() override;
    void invoke(Func &&) override;
    void trigger() override;
    void interrupt() override;
    bool isRunning() override;
    size_t getWorkload() const override;
    void join() override;

private:
    void onThreadUpdate();

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::vector<std::thread> m_threads;
    std::queue<Func> m_queue;
    bool m_isActive;
    uint8_t m_maxNumberOfThreads;
    std::atomic<uint8_t> m_aliveThreads = 0;
    std::map<std::thread::id, comm::Subscription> m_onCrashSubs;
};

} // namespace psi::thread
