
#include <string>
#include <vector>

#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "IPCClientServer_Tests.h"

using namespace psi::test;

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

TEST(IPCClientServer_Tests, callbacks_call_BigArgs)
{
    auto test_fn = MockedFn<ITest::VoidCb::OnCallbackFn>::create();

    run_test_ClientServer([test_fn](TestClient &client, TestServer &) {
        uint8_t arg0 = 'd';
        std::string arg1(3000, 'a');
        std::vector<uint64_t> arg2 = {123, 234, 345};

        EXPECT_CALL(test_fn, 1)
            .WithArgContains(1, "call arguments are too big (3033)");
        client.call_Args_VoidCb(arg0, arg1, arg2, test_fn->fn());
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
