
#include <string>
#include <vector>

#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/IPCCallback.h"
#include "psi/shared/ipc/IPCEvent.h"
#include "psi/shared/ipc/client/IClientIPC.hpp"
#include "psi/shared/ipc/client/IEventClientIPC.h"
#include "psi/shared/ipc/server/IEventServerIPC.h"
#include "psi/shared/ipc/server/IServerIPC.h"

using namespace psi::ipc;
using namespace psi::test;

class ITest
{
public:
    virtual ~ITest() = default;

    enum : uint16_t
    {
        CMD_1 = 1,
        CMD_2,
        CMD_3,
        CMD_4,
        CMD_5,
        CMD_6,
        CMD_7,
        CMD_8,
        CMD_9,
        CMD_A,
        CMD_B,
        CMD_C,
        CMD_D,
        CMD_E,
        CMD_F,
    };

    using VoidCb = IPCCallback<>;
    using ComplexCb = IPCCallback<double, std::vector<uint8_t>, bool, std::vector<std::string>, std::string, int32_t>;
    using VectorCb = IPCCallback<std::vector<uint64_t>>;
    using VectorStringCb = IPCCallback<std::vector<std::string>>;
    virtual void call_NoArgs_NoCb() = 0;
    virtual void call_NoArgs_VoidCb(VoidCb) = 0;
    virtual void call_NoArgs_ComplexCb(ComplexCb) = 0;
    virtual void call_Args_NoCb(double, bool, std::string, int8_t) = 0;
    virtual void call_Args_VoidCb(uint8_t, std::string, std::vector<uint64_t>, VoidCb) = 0;
    virtual void call_Args_ComplexCb(std::vector<std::string>, std::vector<bool>, std::string, uint64_t, ComplexCb) = 0;
    virtual void call_VectorCb(VectorCb) = 0;
    virtual void call_VectorStringCb(VectorStringCb) = 0;
    virtual std::string string_callArgs(long double, bool, std::string, uint16_t) = 0;
    virtual std::vector<std::string> vector_callArgs() = 0;

    enum : uint16_t
    {
        EV_VOID = 0,
        EV_COMPLEX,
        EV_VECTOR,
        EV_VECTOR_STRING,
        EV_COUNT
    };

    using VoidEv = IPCEvent<EV_VOID>;
    using ComplexEv = IPCEvent<EV_COMPLEX, double, bool, std::string, int32_t>;
    using VectorEv = IPCEvent<EV_VECTOR, std::vector<uint64_t>>;
    using VectorStringEv = IPCEvent<EV_VECTOR_STRING, std::vector<std::string>>;
    virtual VoidEv &voidEvent() = 0;
    virtual ComplexEv &complexEvent() = 0;
    virtual VectorEv &vectorEvent() = 0;
    virtual VectorStringEv &vectorStringEvent() = 0;
};

class TestClient : public ITest, public client::IClientIPC
{
public:
    TestClient()
        : client::IClientIPC("TestService", nullptr)
        , m_void_event(*this)
        , m_complex_event(*this)
        , m_vector_event(*this)
        , m_vector_string_event(*this)
    {
    }

    void call_NoArgs_NoCb() override
    {
        INVOKE_SERVER_FN(CMD_1);
    }

    void call_NoArgs_VoidCb(VoidCb arg0) override
    {
        INVOKE_SERVER_FN(CMD_2, arg0);
    }

    void call_NoArgs_ComplexCb(ComplexCb arg0) override
    {
        INVOKE_SERVER_FN(CMD_3, arg0);
    }

    void call_Args_NoCb(double arg0, bool arg1, std::string arg2, int8_t arg3) override
    {
        INVOKE_SERVER_FN(CMD_4, arg0, arg1, arg2, arg3);
    }

    void call_Args_VoidCb(uint8_t arg0, std::string arg1, std::vector<uint64_t> arg2, VoidCb arg3) override
    {
        INVOKE_SERVER_FN(CMD_5, arg0, arg1, arg2, arg3);
    }

    void call_Args_ComplexCb(std::vector<std::string> arg0,
                             std::vector<bool> arg1,
                             std::string arg2,
                             uint64_t arg3,
                             ComplexCb arg4) override
    {
        INVOKE_SERVER_FN(CMD_6, arg0, arg1, arg2, arg3, arg4);
    }

    void call_VectorCb(VectorCb cb) override
    {
        INVOKE_SERVER_FN(CMD_7, cb);
    }

    void call_VectorStringCb(VectorStringCb cb) override
    {
        INVOKE_SERVER_FN(CMD_8, cb);
    }

    std::string string_callArgs(long double arg0, bool arg1, std::string arg2, uint16_t arg3) override
    {
        return INVOKE_SERVER_FN_RETURN<std::string>(CMD_9, arg0, arg1, arg2, arg3);
    }

    std::vector<std::string> vector_callArgs() override
    {
        return INVOKE_SERVER_FN_RETURN<std::vector<std::string>>(CMD_A);
    }

    VoidEv &voidEvent() override
    {
        return m_void_event;
    }

    ComplexEv &complexEvent() override
    {
        return m_complex_event;
    }

    VectorEv &vectorEvent() override
    {
        return m_vector_event;
    }

    VectorStringEv &vectorStringEvent() override
    {
        return m_vector_string_event;
    }

private:
    client::IEventClientIPC<EV_VOID> m_void_event;
    client::IEventClientIPC<EV_COMPLEX, double, bool, std::string, int32_t> m_complex_event;
    client::IEventClientIPC<EV_VECTOR, std::vector<uint64_t>> m_vector_event;
    client::IEventClientIPC<EV_VECTOR_STRING, std::vector<std::string>> m_vector_string_event;
};

class TestServer : public ITest, public server::IServerIPC<TestServer>
{
public:
    TestServer()
        : server::IServerIPC<TestServer>("TestService", nullptr)
        , m_void_event(*this)
        , m_complex_event(*this)
        , m_vector_event(*this)
        , m_vector_string_event(*this)
    {
    }

    TestServer &server() override
    {
        return *this;
    }

    server::FnServiceMap<TestServer> generateFnMap() override
    {
        return {
            {CMD_1, &TestServer::call_NoArgs_NoCb},
            {CMD_2, &TestServer::call_NoArgs_VoidCb},
            {CMD_3, &TestServer::call_NoArgs_ComplexCb},
            {CMD_4, &TestServer::call_Args_NoCb},
            {CMD_5, &TestServer::call_Args_VoidCb},
            {CMD_6, &TestServer::call_Args_ComplexCb},
            {CMD_7, &TestServer::call_VectorCb},
            {CMD_8, &TestServer::call_VectorStringCb},
            {CMD_9, &TestServer::string_callArgs},
            {CMD_A, &TestServer::vector_callArgs},
        };
    }

    void call_NoArgs_NoCb() override
    {
        std::cout << "[server] call_NoArgs_NoCb" << std::endl;
    }

    void call_NoArgs_VoidCb(VoidCb cb) override
    {
        std::cout << "[server] call_NoArgs_VoidCb" << std::endl;
        cb.success();
    }

    void call_NoArgs_ComplexCb(ComplexCb cb) override
    {
        std::cout << "[server] call_NoArgs_ComplexCb" << std::endl;
        cb.success(10.7, {'a', 'b', 'c'}, true, {"a", "bb", "ccc"}, "callback_call_NoArgs_ComplexCb", -10);
    }

    void call_Args_NoCb(double, bool, std::string, int8_t) override
    {
        std::cout << "[server] call_Args_NoCb" << std::endl;
    }

    void call_Args_VoidCb(uint8_t, std::string, std::vector<uint64_t>, VoidCb cb) override
    {
        std::cout << "[server] call_Args_VoidCb" << std::endl;
        cb.success();
    }

    void call_Args_ComplexCb(std::vector<std::string>, std::vector<bool>, std::string, uint64_t, ComplexCb cb) override
    {
        std::cout << "[server] call_Args_ComplexCb" << std::endl;
        cb.success(10.7, {'a', 'b', 'c'}, true, {"a", "bb", "ccc"}, "callback_call_Args_ComplexCb", -10);
    }

    void call_VectorCb(VectorCb cb) override
    {
        std::cout << "[server] call_VectorCb" << std::endl;
        cb.success({1, 2, 3, 4});
    }

    void call_VectorStringCb(VectorStringCb cb) override
    {
        std::cout << "[server] call_VectorStringCb" << std::endl;
        cb.success({"a", "bb", "ccc", "dddd"});
    }

    std::string string_callArgs(long double, bool, std::string, uint16_t) override
    {
        std::cout << "[server] string_callArgs" << std::endl;
        return std::string("abc");
    }

    std::vector<std::string> vector_callArgs() override
    {
        std::cout << "[server] vector_callArgs" << std::endl;
        return {"a", "bb", "ccc", "dddd"};
    }

    VoidEv &voidEvent() override
    {
        return m_void_event;
    }

    ComplexEv &complexEvent() override
    {
        return m_complex_event;
    }

    VectorEv &vectorEvent() override
    {
        return m_vector_event;
    }

    VectorStringEv &vectorStringEvent() override
    {
        return m_vector_string_event;
    }

    void notify_VoidEvent()
    {
        m_void_event.notify();
    }

    void notify_ComplexEvent(double a, bool b, std::string c, int32_t d)
    {
        m_complex_event.notify(a, b, c, d);
    }

    void notify_VectorEvent(std::vector<uint64_t> a)
    {
        m_vector_event.notify(a);
    }

    void notify_VectorStringEvent(std::vector<std::string> a)
    {
        m_vector_string_event.notify(a);
    }

protected:
    server::IEventServerIPC<EV_VOID> m_void_event;
    server::IEventServerIPC<EV_COMPLEX, double, bool, std::string, int32_t> m_complex_event;
    server::IEventServerIPC<EV_VECTOR, std::vector<uint64_t>> m_vector_event;
    server::IEventServerIPC<EV_VECTOR_STRING, std::vector<std::string>> m_vector_string_event;
};

static void run_test_ClientServer(std::function<void(TestClient &, TestServer &)> test_fn)
{
    TestServer server;
    TestClient client;

    server.run();
    client.connect();

    test_fn(client, server);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

TEST(IPCClientServer_Tests, no_callbacks)
{
    run_test_ClientServer([](TestClient &client, TestServer &) {
        client.call_NoArgs_NoCb();
        client.call_Args_NoCb(17.7424624, true, "call_Args_NoCb", -13);
        client.string_callArgs(159.16524165421, true, "", 16);
        client.vector_callArgs();
    });
}

TEST(IPCClientServer_Tests, callbacks_call_NoArgs_VoidCb)
{
    auto test_fn = MockedFn<ITest::VoidCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        EXPECT_CALL(test_fn, 1);
        client.call_NoArgs_VoidCb(test_fn->fn());
    });
}

TEST(IPCClientServer_Tests, callbacks_call_NoArgs_ComplexCb)
{
    auto test_fn = MockedFn<ITest::ComplexCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        EXPECT_CALL(test_fn, 1)
            .WithArgs(0, "", 10.7, {'a', 'b', 'c'}, true, {"a", "bb", "ccc"}, "callback_call_NoArgs_ComplexCb", -10);
        client.call_NoArgs_ComplexCb(test_fn->fn());
    });
}

TEST(IPCClientServer_Tests, callbacks_call_Args_VoidCb)
{
    auto test_fn = MockedFn<ITest::VoidCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        uint8_t arg0 = 'd';
        std::string arg1 = "call_Args_VoidCb";
        std::vector<uint64_t> arg2 = {123, 234, 345};

        EXPECT_CALL(test_fn, 1);
        client.call_Args_VoidCb(arg0, arg1, arg2, test_fn->fn());
    });
}

TEST(IPCClientServer_Tests, callbacks_call_Args_ComplexCb)
{
    auto test_fn = MockedFn<ITest::ComplexCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        std::vector<std::string> arg0 = {"a", "b", "cc"};
        std::vector<bool> arg1 = {true, false, true};
        std::string arg2 = "call_Args_ComplexCb";
        uint64_t arg3 = 13;

        EXPECT_CALL(test_fn, 1)
            .WithArgs(0, "", 10.7, {'a', 'b', 'c'}, true, {"a", "bb", "ccc"}, "callback_call_Args_ComplexCb", -10);
        client.call_Args_ComplexCb(arg0, arg1, arg2, arg3, test_fn->fn());
    });
}

TEST(IPCClientServer_Tests, callbacks_call_VectorCb)
{
    auto test_fn = MockedFn<ITest::VectorCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        EXPECT_CALL(test_fn, 1).WithArgs(0, "", {1, 2, 3, 4});
        client.call_VectorCb(test_fn->fn());
    });
}

TEST(IPCClientServer_Tests, callbacks_call_VectorStringCb)
{
    auto test_fn = MockedFn<ITest::VectorStringCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        EXPECT_CALL(test_fn, 1).WithArgs(0, "", {"a", "bb", "ccc", "dddd"});
        client.call_VectorStringCb(test_fn->fn());
    });
}

TEST(IPCClientServer_Tests, events_voidEvent)
{
    auto on_event_fn = MockedFn<ITest::VoidEv::OnEventUpdateFn>::create();

    run_test_ClientServer([on_event_fn](TestClient &client, TestServer &server) {
        auto sub = client.voidEvent().subscribe(on_event_fn->fn());
        EXPECT_CALL(on_event_fn, 1);
        server.notify_VoidEvent();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
}

TEST(IPCClientServer_Tests, events_complexEvent)
{
    auto on_event_fn = MockedFn<ITest::ComplexEv::OnEventUpdateFn>::create();

    run_test_ClientServer([on_event_fn](TestClient &client, TestServer &server) {
        auto sub = client.complexEvent().subscribe(on_event_fn->fn());
        EXPECT_CALL(on_event_fn, 1).WithArgs(10.7, true, "complex_event", -13);
        server.notify_ComplexEvent(10.7, true, "complex_event", -13);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
}

TEST(IPCClientServer_Tests, events_vectorEvent)
{
    auto on_event_fn = MockedFn<ITest::VectorEv::OnEventUpdateFn>::create();

    run_test_ClientServer([on_event_fn](TestClient &client, TestServer &server) {
        auto sub = client.vectorEvent().subscribe(on_event_fn->fn());
        EXPECT_CALL(on_event_fn, 1).WithArgs({123, 234, 345, 456, 567});
        server.notify_VectorEvent({123, 234, 345, 456, 567});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // auto sub_vecotr_string = client.vectorStringEvent().subscribe();
    });
}

TEST(IPCClientServer_Tests, events_vectorStringEvent)
{
    auto on_event_fn = MockedFn<ITest::VectorStringEv::OnEventUpdateFn>::create();

    run_test_ClientServer([on_event_fn](TestClient &client, TestServer &server) {
        auto sub = client.vectorStringEvent().subscribe(on_event_fn->fn());
        EXPECT_CALL(on_event_fn, 1).WithArgs({"a", "bb", "cccc"});
        server.notify_VectorStringEvent({"a", "bb", "cccc"});
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
}
