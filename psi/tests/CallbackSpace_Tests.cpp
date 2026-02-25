
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/CallbackSpace.h"

using namespace psi::ipc;
using namespace psi::test;

using TestCallbackSpace = CallbackSpace<8, 32>;

TEST(CallbackSpace_Tests, setAvailable_isAvailable)
{
    TestCallbackSpace cs;

    EXPECT_FALSE(cs.isAvailable());

    cs.setAvailable(true);
    EXPECT_TRUE(cs.isAvailable());

    cs.setAvailable(false);
    EXPECT_FALSE(cs.isAvailable());
}

TEST(CallbackSpace_Tests, pushCallback)
{
    // single push
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());
        EXPECT_TRUE(*idx < 8);
    }

    // fill all slots
    {
        TestCallbackSpace cs;

        for (uint16_t i = 0; i < 8; ++i) {
            EXPECT_TRUE(cs.pushCallback().has_value());
        }

        EXPECT_FALSE(cs.pushCallback().has_value()); // overflow
    }
}

TEST(CallbackSpace_Tests, updateCallback)
{
    uint8_t data[4] = {1, 2, 3, 4};

    // normal flow
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        cs.updateCallback(*idx, data, 4);
        EXPECT_TRUE(cs.isCallbackAvailable(*idx));
    }

    // update without push → ignored
    {
        TestCallbackSpace cs;

        cs.updateCallback(0, data, 4);
        EXPECT_FALSE(cs.isCallbackAvailable(0));
    }

    // size overflow → ignored
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        std::array<uint8_t, 33> big {};
        cs.updateCallback(*idx, big.data(), static_cast<uint16_t>(big.size()));

        EXPECT_FALSE(cs.isCallbackAvailable(*idx));
    }

    // invalid index → safe
    {
        TestCallbackSpace cs;
        cs.updateCallback(999, data, 4);
    }
}

TEST(CallbackSpace_Tests, popCallback)
{
    uint8_t data[3] = {9, 8, 7};

    // pop without ready
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        auto view = cs.popCallback(*idx);
        EXPECT_FALSE(view.has_value());
    }

    // normal pop
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        cs.updateCallback(*idx, data, 3);

        auto view = cs.popCallback(*idx);
        ASSERT_TRUE(view.has_value());

        EXPECT_EQ(view->sz, 3);
        EXPECT_EQ(std::memcmp(view->data, data, 3), 0);
    }

    // pop resets slot → can reuse
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        cs.updateCallback(*idx, data, 3);
        ASSERT_TRUE(cs.popCallback(*idx).has_value());

        auto idx2 = cs.pushCallback();
        ASSERT_TRUE(idx2.has_value());
        EXPECT_EQ(idx.value(), idx2.value());
    }

    // invalid index
    {
        TestCallbackSpace cs;

        auto view = cs.popCallback(999);
        EXPECT_FALSE(view.has_value());
    }
}

TEST(CallbackSpace_Tests, clearCallback)
{
    uint8_t data[2] = {5, 5};

    // clear after ready
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        cs.updateCallback(*idx, data, 2);
        cs.clearCallback(*idx);

        auto view = cs.popCallback(*idx);
        EXPECT_FALSE(view.has_value());
    }

    // clear invalid index → safe
    {
        TestCallbackSpace cs;
        cs.clearCallback(999);
    }
}

TEST(CallbackSpace_Tests, isCallbackAvailable)
{
    uint8_t data[1] = {1};

    // not ready
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        EXPECT_FALSE(cs.isCallbackAvailable(*idx));
    }

    // ready
    {
        TestCallbackSpace cs;

        auto idx = cs.pushCallback();
        ASSERT_TRUE(idx.has_value());

        cs.updateCallback(*idx, data, 1);
        EXPECT_TRUE(cs.isCallbackAvailable(*idx));
    }

    // invalid index
    {
        TestCallbackSpace cs;
        EXPECT_FALSE(cs.isCallbackAvailable(999));
    }
}
