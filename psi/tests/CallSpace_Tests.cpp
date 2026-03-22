
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/CallSpace.h"

using namespace psi::ipc;
using namespace psi::test;

//
// ------------------------------------------------------------
// Tested types (defined ONCE)
// ------------------------------------------------------------
//

using CallSpace_Custom = CallSpace<16, 32>;

#define CALLSPACE_TYPES(X)                                                                                             \
    X(CallSpace_Custom)                                                                                                \
    X(CallSpace_Default)                                                                                               \
    X(CallSpace_Q_1024_D_1024)                                                                                         \
    X(CallSpace_Q_1024_D_2048)                                                                                         \
    X(CallSpace_Q_2048_D_512)                                                                                          \
    X(CallSpace_Q_2048_D_1024)                                                                                         \
    X(CallSpace_Q_2048_D_2048)

//
// ------------------------------------------------------------
// Generic test implementations
// ------------------------------------------------------------
//

template <typename Space>
void impl_isAvailable()
{
    auto cs = std::make_unique<Space>();

    EXPECT_FALSE(cs->isAvailable());

    cs->setAvailable(true);
    EXPECT_TRUE(cs->isAvailable());

    cs->setAvailable(false);
    EXPECT_FALSE(cs->isAvailable());
}

template <typename Space>
void impl_isDataAvailable()
{
    auto cs = std::make_unique<Space>();

    EXPECT_FALSE(cs->isDataAvailable());

    uint8_t data[3] = {1, 2, 3};

    EXPECT_TRUE(cs->push(data, 3));
    EXPECT_TRUE(cs->isDataAvailable());

    uint32_t sz = 0;
    const uint8_t *out = cs->pop(sz);

    EXPECT_NE(out, nullptr);
    EXPECT_EQ(sz, 3);

    EXPECT_TRUE(cs->isDataAvailable());

    cs->clear();
    EXPECT_FALSE(cs->isDataAvailable());
}

template <typename Space>
void impl_push()
{
    auto cs = std::make_unique<Space>();

    uint8_t data[4] = {1, 2, 3, 4};

    EXPECT_TRUE(cs->push(data, 4));

    uint32_t sz = 0;
    const uint8_t *out = cs->pop(sz);

    EXPECT_NE(out, nullptr);
    EXPECT_EQ(sz, 4);
    EXPECT_EQ(std::memcmp(out, data, 4), 0);

    cs->clear();
}

template <typename Space>
void impl_pop()
{
    auto cs = std::make_unique<Space>();

    uint8_t data[3] = {9, 8, 7};

    EXPECT_TRUE(cs->push(data, 3));

    uint32_t sz = 0;
    const uint8_t *out = cs->pop(sz);

    EXPECT_NE(out, nullptr);
    EXPECT_EQ(sz, 3);
    EXPECT_EQ(std::memcmp(out, data, 3), 0);
}

template <typename Space>
void impl_generateClientId()
{
    auto cs = std::make_unique<Space>();

    EXPECT_EQ(cs->generateClientId().value(), 1);
    EXPECT_EQ(cs->generateClientId().value(), 2);
    EXPECT_EQ(cs->generateClientId().value(), 3);
}

template <typename Space>
void impl_CALL_SZ()
{
    EXPECT_EQ(Space::CALL_SZ, Space::CALL_SZ);
}

//
// ------------------------------------------------------------
// Test wrappers
// ------------------------------------------------------------
//

#define GENERATE_CALLSPACE_TESTS(space)                                                                                \
    TEST(space##_Tests, isAvailable)                                                                                   \
    {                                                                                                                  \
        impl_isAvailable<space>();                                                                                     \
    }                                                                                                                  \
    TEST(space##_Tests, isDataAvailable)                                                                               \
    {                                                                                                                  \
        impl_isDataAvailable<space>();                                                                                 \
    }                                                                                                                  \
    TEST(space##_Tests, push)                                                                                          \
    {                                                                                                                  \
        impl_push<space>();                                                                                            \
    }                                                                                                                  \
    TEST(space##_Tests, pop)                                                                                           \
    {                                                                                                                  \
        impl_pop<space>();                                                                                             \
    }                                                                                                                  \
    TEST(space##_Tests, generateClientId)                                                                              \
    {                                                                                                                  \
        impl_generateClientId<space>();                                                                                \
    }                                                                                                                  \
    TEST(space##_Tests, CALL_SZ)                                                                                       \
    {                                                                                                                  \
        impl_CALL_SZ<space>();                                                                                         \
    }

CALLSPACE_TYPES(GENERATE_CALLSPACE_TESTS)
