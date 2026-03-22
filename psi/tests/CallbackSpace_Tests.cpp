
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/CallbackSpace.h"

using namespace psi::ipc;
using namespace psi::test;

//
// ------------------------------------------------------------
// Tested types (defined ONCE)
// ------------------------------------------------------------
//

using CallbackSpaceCustom = CallbackSpace<8, 32>;

#define CALLBACK_TYPES(X)                                                                                              \
    X(CallbackSpaceCustom)                                                                                             \
    X(CallbackSpace_Default)                                                                                           \
    X(CallbackSpace_Q_1024_D_1024)                                                                                     \
    X(CallbackSpace_Q_1024_D_2048)                                                                                     \
    X(CallbackSpace_Q_2048_D_512)                                                                                      \
    X(CallbackSpace_Q_2048_D_1024)                                                                                     \
    X(CallbackSpace_Q_2048_D_2048)

//
// ------------------------------------------------------------
// Generic test implementations
// ------------------------------------------------------------
//

template <typename Space>
void impl_setAvailable_isAvailable()
{
    auto cs = std::make_unique<Space>();

    EXPECT_FALSE(cs->isAvailable());

    cs->setAvailable(true);
    EXPECT_TRUE(cs->isAvailable());

    cs->setAvailable(false);
    EXPECT_FALSE(cs->isAvailable());
}

template <typename Space>
void impl_pushCallback()
{
    {
        auto cs = std::make_unique<Space>();

        auto idx = cs->pushCallback();
        ASSERT_TRUE(idx.has_value());
        EXPECT_TRUE(*idx < Space::QUEUE_SIZE);
    }

    {
        auto cs = std::make_unique<Space>();

        for (uint16_t i = 0; i < Space::QUEUE_SIZE; ++i)
            EXPECT_TRUE(cs->pushCallback().has_value());

        EXPECT_FALSE(cs->pushCallback().has_value());
    }
}

template <typename Space>
void impl_updateCallback()
{
    uint8_t data[4] = {1, 2, 3, 4};

    auto cs = std::make_unique<Space>();

    auto idx = cs->pushCallback();
    ASSERT_TRUE(idx.has_value());

    cs->updateCallback(*idx, data, 4);
    EXPECT_TRUE(cs->isCallbackAvailable(*idx));
}

template <typename Space>
void impl_popCallback()
{
    uint8_t data[3] = {9, 8, 7};

    auto cs = std::make_unique<Space>();

    auto idx = cs->pushCallback();
    ASSERT_TRUE(idx.has_value());

    cs->updateCallback(*idx, data, 3);

    auto view = cs->popCallback(*idx);
    ASSERT_TRUE(view.has_value());

    EXPECT_EQ(view->sz, 3);
    EXPECT_EQ(std::memcmp(view->data, data, 3), 0);
}

template <typename Space>
void impl_clearCallback()
{
    uint8_t data[2] = {5, 5};

    auto cs = std::make_unique<Space>();

    auto idx = cs->pushCallback();
    ASSERT_TRUE(idx.has_value());

    cs->updateCallback(*idx, data, 2);
    cs->clearCallback(*idx);

    auto view = cs->popCallback(*idx);
    EXPECT_FALSE(view.has_value());
}

template <typename Space>
void impl_isCallbackAvailable()
{
    uint8_t data[1] = {1};

    auto cs = std::make_unique<Space>();

    auto idx = cs->pushCallback();
    ASSERT_TRUE(idx.has_value());

    EXPECT_FALSE(cs->isCallbackAvailable(*idx));

    cs->updateCallback(*idx, data, 1);
    EXPECT_TRUE(cs->isCallbackAvailable(*idx));
}

//
// ------------------------------------------------------------
// Test wrappers
// ------------------------------------------------------------
//

#define GENERATE_TESTS(space)                                                                                          \
    TEST(space##_Tests, setAvailable_isAvailable)                                                                      \
    {                                                                                                                  \
        impl_setAvailable_isAvailable<space>();                                                                        \
    };                                                                                                                 \
    TEST(space##_Tests, pushCallback)                                                                                  \
    {                                                                                                                  \
        impl_pushCallback<space>();                                                                                    \
    };                                                                                                                 \
    TEST(space##_Tests, updateCallback)                                                                                \
    {                                                                                                                  \
        impl_updateCallback<space>();                                                                                  \
    };                                                                                                                 \
    TEST(space##_Tests, popCallback)                                                                                   \
    {                                                                                                                  \
        impl_popCallback<space>();                                                                                     \
    };                                                                                                                 \
    TEST(space##_Tests, clearCallback)                                                                                 \
    {                                                                                                                  \
        impl_clearCallback<space>();                                                                                   \
    };                                                                                                                 \
    TEST(space##_Tests, isCallbackAvailable)                                                                           \
    {                                                                                                                  \
        impl_isCallbackAvailable<space>();                                                                             \
    };

CALLBACK_TYPES(GENERATE_TESTS)
