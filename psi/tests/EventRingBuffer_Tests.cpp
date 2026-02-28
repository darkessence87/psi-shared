
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/EventRingBuffer.h"

using namespace psi::ipc;
using namespace psi::test;

using TestRB = EventRingBuffer<4, 8>; // capacity = 3 elements

// ============================================================
// push()
// ============================================================

TEST(EventRingBuffer_Tests, push)
{
    TestRB rb;

    uint8_t data[2] = {1, 2};

    // basic push
    EXPECT_TRUE(rb.push(10, data, 2));

    auto v = rb.pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->event_id, 10);
    EXPECT_EQ(v->sz, 2);
    EXPECT_EQ(std::memcmp(v->data, data, 2), 0);

    // reject too large payload
    uint8_t big[9] = {};
    EXPECT_FALSE(rb.push(1, big, 9));

    // zero-length payload allowed
    EXPECT_TRUE(rb.push(2, nullptr, 0));
    auto v0 = rb.pop();
    ASSERT_TRUE(v0.has_value());
    EXPECT_EQ(v0->event_id, 2);
    EXPECT_EQ(v0->sz, 0);

    // fill queue to capacity (3)
    uint8_t x[1] = {7};
    EXPECT_TRUE(rb.push(1, x, 1));
    EXPECT_TRUE(rb.push(2, x, 1));
    EXPECT_TRUE(rb.push(3, x, 1));

    // full → push fails
    EXPECT_FALSE(rb.push(4, x, 1));
}

// ============================================================
// pop()
// ============================================================

TEST(EventRingBuffer_Tests, pop)
{
    TestRB rb;

    // pop on empty
    EXPECT_FALSE(rb.pop().has_value());

    // single push/pop
    uint8_t data[3] = {3, 4, 5};
    EXPECT_TRUE(rb.push(20, data, 3));

    auto v = rb.pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->event_id, 20);
    EXPECT_EQ(v->sz, 3);
    EXPECT_EQ(std::memcmp(v->data, data, 3), 0);

    EXPECT_FALSE(rb.pop().has_value());

    // FIFO order
    uint8_t a[1] = {1};
    uint8_t b[1] = {2};
    uint8_t c[1] = {3};

    EXPECT_TRUE(rb.push(1, a, 1));
    EXPECT_TRUE(rb.push(2, b, 1));
    EXPECT_TRUE(rb.push(3, c, 1));

    auto v1 = rb.pop();
    auto v2 = rb.pop();
    auto v3 = rb.pop();

    ASSERT_TRUE(v1 && v2 && v3);
    EXPECT_EQ(v1->event_id, 1);
    EXPECT_EQ(v2->event_id, 2);
    EXPECT_EQ(v3->event_id, 3);
}

// ============================================================
// wrap-around behavior
// ============================================================

TEST(EventRingBuffer_Tests, wrapAround)
{
    TestRB rb;

    uint8_t data[1] = {9};

    // fill to capacity (3)
    EXPECT_TRUE(rb.push(1, data, 1));
    EXPECT_TRUE(rb.push(2, data, 1));
    EXPECT_TRUE(rb.push(3, data, 1));

    // free two slots
    ASSERT_TRUE(rb.pop().has_value());
    ASSERT_TRUE(rb.pop().has_value());

    // wrap writes
    EXPECT_TRUE(rb.push(4, data, 1));
    EXPECT_TRUE(rb.push(5, data, 1));

    auto v1 = rb.pop();
    auto v2 = rb.pop();
    auto v3 = rb.pop();

    ASSERT_TRUE(v1 && v2 && v3);
    EXPECT_EQ(v1->event_id, 3);
    EXPECT_EQ(v2->event_id, 4);
    EXPECT_EQ(v3->event_id, 5);

    EXPECT_FALSE(rb.pop().has_value());
}

// ============================================================
// reset()
// ============================================================

TEST(EventRingBuffer_Tests, reset)
{
    TestRB rb;

    uint8_t data[2] = {1, 2};

    EXPECT_TRUE(rb.push(10, data, 2));
    EXPECT_TRUE(rb.push(20, data, 2));

    rb.reset();

    // buffer should be empty
    EXPECT_FALSE(rb.pop().has_value());

    // should work normally after reset
    EXPECT_TRUE(rb.push(30, data, 2));

    auto v = rb.pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->event_id, 30);
    EXPECT_EQ(v->sz, 2);
}

// ============================================================
// data pointer semantics
// ============================================================

TEST(EventRingBuffer_Tests, dataPointerPerSlot)
{
    TestRB rb;

    uint8_t a[2] = {1, 2};
    uint8_t b[2] = {3, 4};

    EXPECT_TRUE(rb.push(1, a, 2));
    EXPECT_TRUE(rb.push(2, b, 2));

    auto v1 = rb.pop();
    auto v2 = rb.pop();

    ASSERT_TRUE(v1 && v2);

    // different slots → different pointers
    EXPECT_NE(v1->data, v2->data);

    EXPECT_EQ(std::memcmp(v1->data, a, 2), 0);
    EXPECT_EQ(std::memcmp(v2->data, b, 2), 0);
}
