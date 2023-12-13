
#include "TestHelper.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "psi/thread/Timer.h"
#include "psi/thread/TimerLoop.h"


using namespace ::testing;
using namespace psi::thread;
using namespace psi::test;

struct TimerTests : Test {
    using TimerCallback = std::function<void()>;

    void SetUp()
    {
        m_timerLoop = std::make_shared<TimerLoop>();

        m_timer1 = std::make_shared<Timer>(++m_timerCounter, *m_timerLoop);
        m_timer2 = std::make_shared<Timer>(++m_timerCounter, *m_timerLoop);
        m_timer3 = std::make_shared<Timer>(++m_timerCounter, *m_timerLoop);
        m_timer4 = std::make_shared<Timer>(++m_timerCounter, *m_timerLoop);
        m_timer5 = std::make_shared<Timer>(++m_timerCounter, *m_timerLoop);

        m_timer1Cb = MockedFn<TimerCallback>::create();
        m_timer2Cb = MockedFn<TimerCallback>::create();
        m_timer3Cb = MockedFn<TimerCallback>::create();
        m_timer4Cb = MockedFn<TimerCallback>::create();
        m_timer5Cb = MockedFn<TimerCallback>::create();
    }

    void TearDown() {}

    std::shared_ptr<TimerLoop> m_timerLoop;
    std::atomic<size_t> m_timerCounter;

    std::shared_ptr<Timer> m_timer1;
    std::shared_ptr<Timer> m_timer2;
    std::shared_ptr<Timer> m_timer3;
    std::shared_ptr<Timer> m_timer4;
    std::shared_ptr<Timer> m_timer5;

    std::shared_ptr<MockedFn<TimerCallback>> m_timer1Cb;
    std::shared_ptr<MockedFn<TimerCallback>> m_timer2Cb;
    std::shared_ptr<MockedFn<TimerCallback>> m_timer3Cb;
    std::shared_ptr<MockedFn<TimerCallback>> m_timer4Cb;
    std::shared_ptr<MockedFn<TimerCallback>> m_timer5Cb;
};

// Actions: start(1), stop(2), restart(3), finished(4), running(5)
// Possible 13 cases:
// 1111
// 1f23     = start -> finished -> stop -> restart
// 1r23     = start -> running -> stop -> restart
// 1f3f2    = start -> finished -> restart -> finished -> stop
// 1f3r2    = start -> finished -> restart -> running -> stop
// 1r3f2    = start -> running -> restart -> finished -> stop
// 1r3r2    = start -> running -> restart -> running -> stop
// 21f3     = stop -> start -> finished -> restart
// 21r3     = stop -> start -> running -> restart
// 23_1     = stop -> restart -> finished/running -> start
// 3_1f2    = restart -> finished/running -> start -> finished -> stop
// 3_1r2    = restart -> finished/running -> start -> running -> stop
// 3_21     = restart -> finished/running -> stop -> start

TEST_F(TimerTests, SingleTimer_StartSpam)
{
    InSequence dummy;

    EXPECT_CALL(*m_timer1Cb, f()).Times(1);

    // start spam
    for (int i = 0; i < 100; ++i) {
        m_timer1->start(100, m_timer1Cb->fn());
        EXPECT_TRUE(m_timer1->isRunning());
    }
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StartFinishedStopRestart)
{
    InSequence dummy;

    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StartRunningStopRestart)
{
    InSequence dummy;

    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StartFinishedRestartFinishedStop)
{
    InSequence dummy;

    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->restart();
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StartFinishedRestartRunningStop)
{
    InSequence dummy;

    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StartRunningRestartFinishedStop)
{
    InSequence dummy;

    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->restart();
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StartRunningRestartRunningStop)
{
    InSequence dummy;

    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StopStartFinishedRestart)
{
    InSequence dummy;

    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->restart();
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StopStartRunningRestart)
{
    InSequence dummy;

    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->restart();
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_StopRestartStart)
{
    InSequence dummy;

    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    EXPECT_FALSE(m_timer1->isRunning());
    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_RestartStartFinishedStop)
{
    InSequence dummy;

    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    EXPECT_FALSE(m_timer1->isRunning());
    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_RestartStartRunningStop)
{
    InSequence dummy;

    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    EXPECT_FALSE(m_timer1->isRunning());
    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, SingleTimer_RestartStopStart)
{
    InSequence dummy;

    // restart
    EXPECT_CALL(*m_timer1Cb, f()).Times(0);
    m_timer1->restart();
    EXPECT_FALSE(m_timer1->isRunning());
    // stop
    m_timer1->stop();
    EXPECT_FALSE(m_timer1->isRunning());
    // start
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    m_timer1->start(100, m_timer1Cb->fn());
    // running
    EXPECT_TRUE(m_timer1->isRunning());
    // finished
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    EXPECT_FALSE(m_timer1->isRunning());
}

TEST_F(TimerTests, MultipleTimers_OrderByFastest)
{
    InSequence dummy;

    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    EXPECT_CALL(*m_timer2Cb, f()).Times(1);
    EXPECT_CALL(*m_timer3Cb, f()).Times(1);
    EXPECT_CALL(*m_timer4Cb, f()).Times(1);
    EXPECT_CALL(*m_timer5Cb, f()).Times(1);

    // start in order by fastest timer
    m_timer1->start(100, m_timer1Cb->fn());
    m_timer2->start(110, m_timer2Cb->fn());
    m_timer3->start(120, m_timer3Cb->fn());
    m_timer4->start(130, m_timer4Cb->fn());
    m_timer5->start(140, m_timer5Cb->fn());

    EXPECT_TRUE(m_timer1->isRunning());
    EXPECT_TRUE(m_timer2->isRunning());
    EXPECT_TRUE(m_timer3->isRunning());
    EXPECT_TRUE(m_timer4->isRunning());
    EXPECT_TRUE(m_timer5->isRunning());

    // finished all
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(m_timer1->isRunning());
    EXPECT_FALSE(m_timer2->isRunning());
    EXPECT_FALSE(m_timer3->isRunning());
    EXPECT_FALSE(m_timer4->isRunning());
    EXPECT_FALSE(m_timer5->isRunning());
}

TEST_F(TimerTests, MultipleTimers_OrderBySlowest)
{
    InSequence dummy;

    EXPECT_CALL(*m_timer5Cb, f()).Times(1);
    EXPECT_CALL(*m_timer4Cb, f()).Times(1);
    EXPECT_CALL(*m_timer3Cb, f()).Times(1);
    EXPECT_CALL(*m_timer2Cb, f()).Times(1);
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);

    // start in order by fastest timer
    m_timer1->start(140, m_timer1Cb->fn());
    m_timer2->start(130, m_timer2Cb->fn());
    m_timer3->start(120, m_timer3Cb->fn());
    m_timer4->start(110, m_timer4Cb->fn());
    m_timer5->start(100, m_timer5Cb->fn());

    EXPECT_TRUE(m_timer1->isRunning());
    EXPECT_TRUE(m_timer2->isRunning());
    EXPECT_TRUE(m_timer3->isRunning());
    EXPECT_TRUE(m_timer4->isRunning());
    EXPECT_TRUE(m_timer5->isRunning());

    // finished all
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(m_timer1->isRunning());
    EXPECT_FALSE(m_timer2->isRunning());
    EXPECT_FALSE(m_timer3->isRunning());
    EXPECT_FALSE(m_timer4->isRunning());
    EXPECT_FALSE(m_timer5->isRunning());
}

TEST_F(TimerTests, MultipleTimers_OrderByFastestRestartToNonEqualTime)
{
    InSequence dummy;

    EXPECT_CALL(*m_timer2Cb, f()).Times(1);
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    EXPECT_CALL(*m_timer4Cb, f()).Times(1);
    EXPECT_CALL(*m_timer3Cb, f()).Times(1);
    EXPECT_CALL(*m_timer5Cb, f()).Times(1);

    // start in order by fastest timer
    m_timer1->start(1000, m_timer1Cb->fn());
    m_timer2->start(1500, m_timer2Cb->fn());
    m_timer3->start(2000, m_timer3Cb->fn());
    m_timer4->start(2500, m_timer4Cb->fn());
    m_timer5->start(3000, m_timer5Cb->fn());

    EXPECT_TRUE(m_timer1->isRunning());
    EXPECT_TRUE(m_timer2->isRunning());
    EXPECT_TRUE(m_timer3->isRunning());
    EXPECT_TRUE(m_timer4->isRunning());
    EXPECT_TRUE(m_timer5->isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(750));
    m_timer1->restart(); // 1750
                         // 1500
    m_timer3->restart(); // 2750
                         // 2500
    m_timer5->restart(); // 3750

    // finished all
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    EXPECT_FALSE(m_timer1->isRunning());
    EXPECT_FALSE(m_timer2->isRunning());
    EXPECT_FALSE(m_timer3->isRunning());
    EXPECT_FALSE(m_timer4->isRunning());
    EXPECT_FALSE(m_timer5->isRunning());
}

TEST_F(TimerTests, MultipleTimers_OrderByFastestRestartToEqualTime)
{
    InSequence dummy;

    EXPECT_CALL(*m_timer2Cb, f()).Times(1);
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);
    EXPECT_CALL(*m_timer4Cb, f()).Times(1);
    EXPECT_CALL(*m_timer3Cb, f()).Times(1);
    EXPECT_CALL(*m_timer5Cb, f()).Times(1);

    // start in order by fastest timer
    m_timer1->start(1000, m_timer1Cb->fn());
    m_timer2->start(1500, m_timer2Cb->fn());
    m_timer3->start(2000, m_timer3Cb->fn());
    m_timer4->start(2500, m_timer4Cb->fn());
    m_timer5->start(3000, m_timer5Cb->fn());

    EXPECT_TRUE(m_timer1->isRunning());
    EXPECT_TRUE(m_timer2->isRunning());
    EXPECT_TRUE(m_timer3->isRunning());
    EXPECT_TRUE(m_timer4->isRunning());
    EXPECT_TRUE(m_timer5->isRunning());

    // sleep 50 ms -> restart
    std::this_thread::sleep_for(std::chrono::milliseconds(510));
    m_timer1->restart(); // 1510
                         // 1500
    m_timer3->restart(); // 2510
                         // 2500
    m_timer5->restart(); // 3510

    // finished all
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    EXPECT_FALSE(m_timer1->isRunning());
    EXPECT_FALSE(m_timer2->isRunning());
    EXPECT_FALSE(m_timer3->isRunning());
    EXPECT_FALSE(m_timer4->isRunning());
    EXPECT_FALSE(m_timer5->isRunning());
}

TEST_F(TimerTests, MultipleTimers_OrderBySlowestRestartToNonEqualTime)
{
    InSequence dummy;

    EXPECT_CALL(*m_timer4Cb, f()).Times(1);
    EXPECT_CALL(*m_timer5Cb, f()).Times(1);
    EXPECT_CALL(*m_timer2Cb, f()).Times(1);
    EXPECT_CALL(*m_timer3Cb, f()).Times(1);
    EXPECT_CALL(*m_timer1Cb, f()).Times(1);

    // start in order by slowest timer
    m_timer1->start(3000, m_timer1Cb->fn());
    m_timer2->start(2500, m_timer2Cb->fn());
    m_timer3->start(2000, m_timer3Cb->fn());
    m_timer4->start(1500, m_timer4Cb->fn());
    m_timer5->start(1000, m_timer5Cb->fn());

    EXPECT_TRUE(m_timer1->isRunning());
    EXPECT_TRUE(m_timer2->isRunning());
    EXPECT_TRUE(m_timer3->isRunning());
    EXPECT_TRUE(m_timer4->isRunning());
    EXPECT_TRUE(m_timer5->isRunning());

    std::this_thread::sleep_for(std::chrono::milliseconds(750));
    m_timer1->restart(); // 3750
                         // 2500
    m_timer3->restart(); // 2750
                         // 1500
    m_timer5->restart(); // 1750

    // finished all
    std::this_thread::sleep_for(std::chrono::milliseconds(3500));
    EXPECT_FALSE(m_timer1->isRunning());
    EXPECT_FALSE(m_timer2->isRunning());
    EXPECT_FALSE(m_timer3->isRunning());
    EXPECT_FALSE(m_timer4->isRunning());
    EXPECT_FALSE(m_timer5->isRunning());
}
