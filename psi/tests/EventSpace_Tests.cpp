
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/EventSpace.h"

using namespace psi::ipc;
using namespace psi::test;

//
// ------------------------------------------------------------
// Tested types (defined ONCE)
// ------------------------------------------------------------
//

using EventSpace_Custom = EventSpace<4>;

#define EVENTSPACE_TYPES(X)                                                                                            \
    X(EventSpace_Custom)                                                                                               \
    X(EventSpace_Default)                                                                                              \
    X(EventSpace_C_32)                                                                                                 \
    X(EventSpace_C_64)

//
// ------------------------------------------------------------
// Generic test implementations
// ------------------------------------------------------------
//

template <typename Space>
void impl_availability()
{
    auto es = std::make_unique<Space>();

    EXPECT_FALSE(es->isAvailable());

    es->setAvailable(true);
    EXPECT_TRUE(es->isAvailable());

    es->setAvailable(false);
    EXPECT_FALSE(es->isAvailable());
}

template <typename Space>
void impl_registerClient()
{
    auto es = std::make_unique<Space>();

    std::vector<uint16_t> ids;

    for (uint16_t i = 0; i < Space::CLIENTS_SIZE; ++i) {
        auto c = es->registerClient();
        ASSERT_TRUE(c.has_value());
        EXPECT_EQ(*c, i);
        ids.push_back(*c);
    }

    // no more clients allowed
    EXPECT_FALSE(es->registerClient().has_value());
}

template <typename Space>
void impl_unregisterClient()
{
    auto es = std::make_unique<Space>();

    auto c0 = es->registerClient();
    ASSERT_TRUE(c0.has_value());

    es->unregisterClient(*c0);

    auto c1 = es->registerClient();
    ASSERT_TRUE(c1.has_value());
    EXPECT_EQ(*c1, *c0);

    es->unregisterClient(999);
}

template <typename Space>
void impl_push()
{
    auto es = std::make_unique<Space>();

    uint8_t data[2] = {1, 2};

    EXPECT_FALSE(es->push(1, data, 2));

    es->setAvailable(true);

    EXPECT_FALSE(es->push(1, data, 2));

    auto c0 = es->registerClient();
    auto c1 = es->registerClient();

    ASSERT_TRUE(c0.has_value());
    ASSERT_TRUE(c1.has_value());

    EXPECT_TRUE(es->push(5, data, 2));

    auto v0 = es->pop(*c0);
    auto v1 = es->pop(*c1);

    ASSERT_TRUE(v0.has_value());
    ASSERT_TRUE(v1.has_value());

    EXPECT_EQ(v0->event_id, 5);
    EXPECT_EQ(v1->event_id, 5);

    EXPECT_EQ(v0->sz, 2);
    EXPECT_EQ(v1->sz, 2);

    EXPECT_EQ(std::memcmp(v0->data, data, 2), 0);
    EXPECT_EQ(std::memcmp(v1->data, data, 2), 0);
}

template <typename Space>
void impl_pop()
{
    auto es = std::make_unique<Space>();
    es->setAvailable(true);

    uint8_t data[3] = {3, 4, 5};

    auto c0 = es->registerClient();
    ASSERT_TRUE(c0.has_value());

    EXPECT_FALSE(es->pop(*c0).has_value());

    EXPECT_TRUE(es->push(10, data, 3));

    auto v = es->pop(*c0);
    ASSERT_TRUE(v.has_value());

    EXPECT_EQ(v->event_id, 10);
    EXPECT_EQ(v->sz, 3);
    EXPECT_EQ(std::memcmp(v->data, data, 3), 0);

    EXPECT_FALSE(es->pop(*c0).has_value());
}

template <typename Space>
void impl_pop_invalidClient()
{
    auto es = std::make_unique<Space>();
    es->setAvailable(true);

    EXPECT_FALSE(es->pop(0).has_value());
    EXPECT_FALSE(es->pop(999).has_value());
}

template <typename Space>
void impl_clientIsolation()
{
    auto es = std::make_unique<Space>();
    es->setAvailable(true);

    uint8_t a[1] = {1};
    uint8_t b[1] = {2};

    auto c0 = es->registerClient();
    auto c1 = es->registerClient();

    ASSERT_TRUE(c0.has_value());
    ASSERT_TRUE(c1.has_value());

    EXPECT_TRUE(es->push(1, a, 1));
    EXPECT_TRUE(es->push(2, b, 1));

    auto v0_1 = es->pop(*c0);
    auto v0_2 = es->pop(*c0);

    ASSERT_TRUE(v0_1.has_value());
    ASSERT_TRUE(v0_2.has_value());

    EXPECT_EQ(v0_1->event_id, 1);
    EXPECT_EQ(v0_2->event_id, 2);

    auto v1_1 = es->pop(*c1);
    auto v1_2 = es->pop(*c1);

    ASSERT_TRUE(v1_1.has_value());
    ASSERT_TRUE(v1_2.has_value());

    EXPECT_EQ(v1_1->event_id, 1);
    EXPECT_EQ(v1_2->event_id, 2);
}

template <typename Space>
void impl_unregisterResetsQueue()
{
    auto es = std::make_unique<Space>();
    es->setAvailable(true);

    uint8_t data[1] = {7};

    auto c0 = es->registerClient();
    ASSERT_TRUE(c0.has_value());

    EXPECT_TRUE(es->push(1, data, 1));

    es->unregisterClient(*c0);

    auto c1 = es->registerClient();
    ASSERT_TRUE(c1.has_value());

    EXPECT_EQ(*c1, *c0);

    EXPECT_FALSE(es->pop(*c1).has_value());
}

//
// ------------------------------------------------------------
// Test wrappers
// ------------------------------------------------------------
//

#define GENERATE_EVENTSPACE_TESTS(space)                                                                               \
    TEST(space##_Tests, availability)                                                                                  \
    {                                                                                                                  \
        impl_availability<space>();                                                                                    \
    }                                                                                                                  \
    TEST(space##_Tests, registerClient)                                                                                \
    {                                                                                                                  \
        impl_registerClient<space>();                                                                                  \
    }                                                                                                                  \
    TEST(space##_Tests, unregisterClient)                                                                              \
    {                                                                                                                  \
        impl_unregisterClient<space>();                                                                                \
    }                                                                                                                  \
    TEST(space##_Tests, push)                                                                                          \
    {                                                                                                                  \
        impl_push<space>();                                                                                            \
    }                                                                                                                  \
    TEST(space##_Tests, pop)                                                                                           \
    {                                                                                                                  \
        impl_pop<space>();                                                                                             \
    }                                                                                                                  \
    TEST(space##_Tests, pop_invalidClient)                                                                             \
    {                                                                                                                  \
        impl_pop_invalidClient<space>();                                                                               \
    }                                                                                                                  \
    TEST(space##_Tests, clientIsolation)                                                                               \
    {                                                                                                                  \
        impl_clientIsolation<space>();                                                                                 \
    }                                                                                                                  \
    TEST(space##_Tests, unregisterResetsQueue)                                                                         \
    {                                                                                                                  \
        impl_unregisterResetsQueue<space>();                                                                           \
    }

EVENTSPACE_TYPES(GENERATE_EVENTSPACE_TESTS)
