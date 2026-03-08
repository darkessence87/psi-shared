
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/EventSpace.h"

using namespace psi::ipc;
using namespace psi::test;

using TestES = EventSpace<4>;

// ============================================================
// availability
// ============================================================

TEST(EventSpace_Tests, availability)
{
    TestES es;

    EXPECT_FALSE(es.isAvailable());

    es.setAvailable(true);
    EXPECT_TRUE(es.isAvailable());

    es.setAvailable(false);
    EXPECT_FALSE(es.isAvailable());
}

// ============================================================
// registerClient
// ============================================================

TEST(EventSpace_Tests, registerClient)
{
    TestES es;

    auto c0 = es.registerClient();
    auto c1 = es.registerClient();
    auto c2 = es.registerClient();
    auto c3 = es.registerClient();

    ASSERT_TRUE(c0.has_value());
    ASSERT_TRUE(c1.has_value());
    ASSERT_TRUE(c2.has_value());
    ASSERT_TRUE(c3.has_value());

    EXPECT_EQ(*c0, 0);
    EXPECT_EQ(*c1, 1);
    EXPECT_EQ(*c2, 2);
    EXPECT_EQ(*c3, 3);

    EXPECT_FALSE(es.registerClient().has_value());
}

// ============================================================
// unregisterClient
// ============================================================

TEST(EventSpace_Tests, unregisterClient)
{
    TestES es;

    auto c0 = es.registerClient();
    ASSERT_TRUE(c0.has_value());

    es.unregisterClient(*c0);

    auto c1 = es.registerClient();
    ASSERT_TRUE(c1.has_value());
    EXPECT_EQ(*c1, *c0);

    es.unregisterClient(999);
}

// ============================================================
// push
// ============================================================

TEST(EventSpace_Tests, push)
{
    TestES es;

    uint8_t data[2] = {1, 2};

    EXPECT_FALSE(es.push(1, data, 2)); // not available

    es.setAvailable(true);

    EXPECT_FALSE(es.push(1, data, 2)); // no clients

    auto c0 = es.registerClient();
    auto c1 = es.registerClient();
    ASSERT_TRUE(c0.has_value());
    ASSERT_TRUE(c1.has_value());

    EXPECT_TRUE(es.push(5, data, 2));

    auto v0 = es.pop(*c0);
    auto v1 = es.pop(*c1);

    ASSERT_TRUE(v0.has_value());
    ASSERT_TRUE(v1.has_value());

    EXPECT_EQ(v0->event_id, 5);
    EXPECT_EQ(v1->event_id, 5);

    EXPECT_EQ(v0->sz, 2);
    EXPECT_EQ(v1->sz, 2);

    EXPECT_EQ(std::memcmp(v0->data, data, 2), 0);
    EXPECT_EQ(std::memcmp(v1->data, data, 2), 0);
}

// ============================================================
// push partial clients
// ============================================================

TEST(EventSpace_Tests, push_partialClients)
{
    TestES es;
    es.setAvailable(true);

    uint8_t data[1] = {9};

    auto c0 = es.registerClient();
    auto c1 = es.registerClient();
    ASSERT_TRUE(c0.has_value());
    ASSERT_TRUE(c1.has_value());

    for (int i = 0; i < 127; ++i) {
        EXPECT_TRUE(es.push(1, data, 1));
        ASSERT_TRUE(es.pop(*c1).has_value());
    }

    EXPECT_TRUE(es.push(2, data, 1));

    auto v0 = es.pop(*c0);
    ASSERT_TRUE(v0.has_value());
    EXPECT_EQ(v0->event_id, 1);

    auto v1 = es.pop(*c1);
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(v1->event_id, 2);
}

// ============================================================
// pop
// ============================================================

TEST(EventSpace_Tests, pop)
{
    TestES es;
    es.setAvailable(true);

    uint8_t data[3] = {3, 4, 5};

    auto c0 = es.registerClient();
    ASSERT_TRUE(c0.has_value());

    EXPECT_FALSE(es.pop(*c0).has_value());

    EXPECT_TRUE(es.push(10, data, 3));

    auto v = es.pop(*c0);
    ASSERT_TRUE(v.has_value());

    EXPECT_EQ(v->event_id, 10);
    EXPECT_EQ(v->sz, 3);
    EXPECT_EQ(std::memcmp(v->data, data, 3), 0);

    EXPECT_FALSE(es.pop(*c0).has_value());
}

// ============================================================
// pop invalid client
// ============================================================

TEST(EventSpace_Tests, pop_invalidClient)
{
    TestES es;
    es.setAvailable(true);

    EXPECT_FALSE(es.pop(0).has_value());
    EXPECT_FALSE(es.pop(999).has_value());
}

// ============================================================
// client isolation
// ============================================================

TEST(EventSpace_Tests, clientIsolation)
{
    TestES es;
    es.setAvailable(true);

    uint8_t a[1] = {1};
    uint8_t b[1] = {2};

    auto c0 = es.registerClient();
    auto c1 = es.registerClient();
    ASSERT_TRUE(c0.has_value());
    ASSERT_TRUE(c1.has_value());

    EXPECT_TRUE(es.push(1, a, 1));
    EXPECT_TRUE(es.push(2, b, 1));

    auto v0_1 = es.pop(*c0);
    auto v0_2 = es.pop(*c0);

    ASSERT_TRUE(v0_1.has_value());
    ASSERT_TRUE(v0_2.has_value());

    EXPECT_EQ(v0_1->event_id, 1);
    EXPECT_EQ(v0_2->event_id, 2);

    auto v1_1 = es.pop(*c1);
    auto v1_2 = es.pop(*c1);

    ASSERT_TRUE(v1_1.has_value());
    ASSERT_TRUE(v1_2.has_value());

    EXPECT_EQ(v1_1->event_id, 1);
    EXPECT_EQ(v1_2->event_id, 2);
}

// ============================================================
// unregister resets queue
// ============================================================

TEST(EventSpace_Tests, unregisterResetsQueue)
{
    TestES es;
    es.setAvailable(true);

    uint8_t data[1] = {7};

    auto c0 = es.registerClient();
    ASSERT_TRUE(c0.has_value());

    EXPECT_TRUE(es.push(1, data, 1));

    es.unregisterClient(*c0);

    auto c1 = es.registerClient();
    ASSERT_TRUE(c1.has_value());
    EXPECT_EQ(*c1, *c0);

    EXPECT_FALSE(es.pop(*c1).has_value());
}
