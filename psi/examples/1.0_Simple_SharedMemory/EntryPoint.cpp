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
    void callNoArgsNoCb() override
    {
        LOG_INFO("server: [callNoArgsNoCb]");
    }
    void callArgsNoCb(double d, bool b, std::string s, int8_t i) override
    {
        LOG_INFO("server: [callArgsNoCb] d: " << d << ", b: " << b << ", s: " << s << ", i: " << i);
    }
    void callNoArgsVoidCb(VoidCb cb) override
    {
        LOG_INFO("server: [callNoArgsVoidCb]");
        cb.success();
    }
    void callNoArgsComplexCb(ComplexCb cb) override
    {
        LOG_INFO("server: [callNoArgsComplexCb]");
        cb.success(15.7, true, "callNoArgsComplexCb", -10);
    }
    void callArgsCb(long double ld, bool b, std::string s, uint64_t u, ComplexCb cb) override
    {
        LOG_INFO("server: [callArgsCb] ld: " << ld << ", b: " << b << ", s: " << s << ", u: " << std::to_string(u));
        cb.success(7.15, true, "callArgsCb", -7);
    }
    std::string stringCallArgs(long double ld, bool b, std::string s, uint16_t u) override
    {
        LOG_INFO("server: [stringCallArgs] ld: " << ld << ", b: " << b << ", s: " << s << ", u: " << std::to_string(u));
        return "stringCallArgs";
    }
};

// -------------------------------------------------------------

void startBenchmark_callbacks(TestClient &client)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto testFn = [&]() {
        const int ITERATIONS = 10000;
        std::vector<size_t> stat;
        stat.resize(ITERATIONS);
        std::atomic<int> finishedTasks = 0;

        for (auto i = 0; i < ITERATIONS; ++i) {
            const auto &start = high_resolution_clock::now();
            client.callArgsCb(54321.12345,
                              true,
                              "test",
                              i,
                              [start, &stat, &finishedTasks, i](uint16_t, std::string, double, bool, std::string, int32_t) {
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

void startBenchmark_events(TestClient &client, TestServer &server)
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto testFn = [&]() {
        const int ITERATIONS = 100;
        std::atomic<int> finishedTasks = 0;

        auto client_complex_sub = client.complexEvent().subscribe([&finishedTasks](auto a, auto b, auto c, auto d) {
            ++finishedTasks;
            std::cout << "[" << finishedTasks << "] [complexEvent] received: [" << a << " " << b << " " << c << " " << d
                      << "]" << std::endl;
        });

        for (auto i = 0; i < ITERATIONS; ++i) {
            server.notify_ComplexEvent(10.13, true, "cool_msg", -7);
        }

        while (finishedTasks < ITERATIONS) {
            std::this_thread::sleep_for(10ms);
        }
    };

    testFn();
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) {
            os << ", ";
        }
        os << v[i];
    }
    os << "]";
    return os;
}

template <typename... Args>
void debug_print(Args &&...args)
{
    ((std::cout << args << " "), ...);
    std::cout << std::endl;
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

        // {
        //     const auto &start = high_resolution_clock::now();
        //     startBenchmark_callbacks(client);
        //     const auto &end = high_resolution_clock::now();
        //     std::cout << "[total], ms: " << (end - start).count() / 1000000.0 << std::endl;
        // }

        // {
        //     const auto &start = high_resolution_clock::now();
        //     startBenchmark_events(client, server);
        //     const auto &end = high_resolution_clock::now();
        //     std::cout << "[total], ms: " << (end - start).count() / 1000000.0 << std::endl;
        // }

        client.callArgsCb(10.1, true, "hello", 123, [](auto... args) { debug_print("client: [callback callArgsCb]", args...); });
        client.callArgsNoCb(10.2, true, "hello2", 135);
        client.callNoArgsComplexCb([](auto... args) { debug_print("client: [callback callNoArgsComplexCb]", args...); });
        client.callNoArgsNoCb();
        client.callNoArgsVoidCb([](auto... args) { debug_print("client: [callback callNoArgsVoidCb]", args...); });
        client.callVectorCb([](auto... args) { debug_print("client: [callback callVectorCb]", args...); });
        client.callVectorStringCb([](auto... args) { debug_print("client: [callback callVectorStringCb]", args...); });
        auto sub0 = client.complexEvent().subscribe([](auto... args) { debug_print("client: [complexEvent]", args...); });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "main end" << std::endl;

    return 0;
}
