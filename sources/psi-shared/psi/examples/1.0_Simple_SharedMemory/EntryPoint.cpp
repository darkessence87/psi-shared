#include "TestClient.h"
#include "TestServer.h"

#include <chrono>

#ifdef PSI_LOGGER
#include "psi/logger/Logger.h"
#else
#include <iostream>
#include <sstream>
#define LOG_INFO(x)                                                                                                    \
    do {                                                                                                               \
        std::ostringstream os;                                                                                         \
        os << x;                                                                                                       \
        std::cout << os.str() << std::endl;                                                                            \
    } while (0)
#endif

using namespace psi;
using namespace psi::examples;

// -------------------------------------------------------------
class TestService : public TestServer
{
public:
    TestService(std::shared_ptr<thread::ILoop> loop = nullptr)
        : TestServer(loop)
    {
    }
    void callNoArgsNoCb()
    {
        LOG_INFO("server: [callNoArgsNoCb]");
    }
    void callArgsNoCb(double d, bool b, std::string s, int8_t i)
    {
        LOG_INFO("server: [callArgsNoCb] d: " << d << ", b: " << b << ", s: " << s << ", i: " << i);
    }
    void callNoArgsVoidCb(VoidCb cb)
    {
        LOG_INFO("server: [callNoArgsVoidCb]");
        cb();
    }
    void callNoArgsComplexCb(ComplexCb cb)
    {
        LOG_INFO("server: [callNoArgsComplexCb]");
        cb(15.7, true, "callNoArgsComplexCb", -10);
    }
    void callArgsCb(long double ld, bool b, std::string s, uint64_t u, ComplexCb cb)
    {
        LOG_INFO("server: [callArgsCb] ld: " << ld << ", b: " << b << ", s: " << s << ", u: " << std::to_string(u));
        cb(7.15, true, "callArgsCb", -7);
    }
    std::string stringCallArgs(long double ld, bool b, std::string s, uint16_t u)
    {
        LOG_INFO("server: [stringCallArgs] ld: " << ld << ", b: " << b << ", s: " << s << ", u: " << std::to_string(u));
        return "stringCallArgs";
    }
};

// -------------------------------------------------------------

void startBenchmark(TestClient &client)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto testFn = [&client]() {
        const int ITERATIONS = 10000;
        std::vector<size_t> stat;
        stat.resize(ITERATIONS);
        std::atomic<int> finishedTasks = 0;

        for (auto i = 0; i < ITERATIONS; ++i) {
            const auto &start = high_resolution_clock::now();
            client.callArgsCb(54321.12345, true, "test", i, [start, &stat, &finishedTasks, i](double, bool, std::string, int32_t) {
                const auto &end = high_resolution_clock::now();
                stat[i] = (end - start).count();
                ++finishedTasks;
            });
        }

        while (finishedTasks < ITERATIONS) {
            std::this_thread::sleep_for(10ms);
        }

        return stat;
    };

    const auto &stats = testFn();

    long double totalTime = 0.0;
    for (const auto &s : stats) {
        totalTime += s;
    }
    std::cout << "[callArgsCb], average ms: " << totalTime / 1'000'000.0 / stats.size() << std::endl;
}

int main()
{
    using namespace std::chrono;

    std::cout << "main start" << std::endl;

    {
        TestClient client;
        client.connect();

        TestService server;
        server.run();

        const auto &start = high_resolution_clock::now();
        startBenchmark(client);
        const auto &end = high_resolution_clock::now();
        std::cout << "[total], ms: " << (end - start).count() / 1000000.0 << std::endl;
    }

    std::cout << "main end" << std::endl;

    return 0;
}
