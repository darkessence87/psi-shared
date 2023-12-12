#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "psi/comm/Subscription.h"
#include "psi/thread/ILoop.h"

namespace psi::thread {

class ThreadPoolQueued : public ILoop
{
    class SimpleThread
    {
    public:
        SimpleThread();
        virtual ~SimpleThread();

        void run();
        void invoke(Func &&);
        void trigger();
        void interrupt();
        bool isRunning();
        size_t getWorkload() const;
        void join();

        void onThreadUpdate();

    private:
        SimpleThread(const SimpleThread &) = delete;
        SimpleThread &operator=(const SimpleThread &) = delete;

    private:
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<Func> m_queue;
        bool m_isActive;
        std::thread m_thread;

        psi::comm::Subscription m_crashSub;
    };

public:
    ThreadPoolQueued(uint8_t numberOfThreads = 10);
    virtual ~ThreadPoolQueued();

public: // ILoop implementation
    void run() override;
    void invoke(Func &&) override;
    void trigger() override;
    void interrupt() override;
    bool isRunning() override;
    size_t getWorkload() const override;
    void join() override;

private:
    std::atomic<int> m_threadIndex;
    std::vector<std::unique_ptr<SimpleThread>> m_threads;
    uint8_t m_maxNumberOfThreads;
};

} // namespace psi::thread
