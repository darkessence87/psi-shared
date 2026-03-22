
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/EventRingBuffer.h"

using namespace psi::ipc;
using namespace psi::test;

//
// ------------------------------------------------------------
// Tested types
// ------------------------------------------------------------
//

using EventRingBufferCustom = EventRingBuffer<4, 8>;
using EventRingBufferDefault = EventRingBuffer<128, 256>;

#define EVENT_RB_TYPES(X)                                                                                              \
    X(EventRingBufferCustom)                                                                                           \
    X(EventRingBufferDefault)

//
// ------------------------------------------------------------
// Generic test implementations
// ------------------------------------------------------------
//

template <typename RB>
void impl_push()
{
    RB rb;

    uint8_t data[2] = {1, 2};

    EXPECT_TRUE(rb.push(10, data, 2));

    auto v = rb.pop();
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->event_id, 10);
    EXPECT_EQ(v->sz, 2);
    EXPECT_EQ(std::memcmp(v->data, data, 2), 0);

    uint8_t big[RB::DATA_SIZE + 1] = {};
    EXPECT_FALSE(rb.push(1, big, RB::DATA_SIZE + 1));

    EXPECT_TRUE(rb.push(2, nullptr, 0));

    auto v0 = rb.pop();
    ASSERT_TRUE(v0.has_value());
    EXPECT_EQ(v0->event_id, 2);
    EXPECT_EQ(v0->sz, 0);

    uint8_t x[1] = {7};

    constexpr size_t capacity = RB::QUEUE_SIZE - 1;

    for (size_t i = 0; i < capacity; ++i) {
        EXPECT_TRUE(rb.push(static_cast<uint16_t>(i), x, 1));
    }

    EXPECT_FALSE(rb.push(999, x, 1)); // full
}

template <typename RB>
void impl_pop()
{
    RB rb;

    EXPECT_FALSE(rb.pop().has_value());

    uint8_t data[3] = {3, 4, 5};
    EXPECT_TRUE(rb.push(20, data, 3));

    auto v = rb.pop();

    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->event_id, 20);
    EXPECT_EQ(v->sz, 3);
    EXPECT_EQ(std::memcmp(v->data, data, 3), 0);

    EXPECT_FALSE(rb.pop().has_value());

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

template <typename RB>
void impl_wrapAround()
{
    RB rb;

    uint8_t data[1] = {9};

    EXPECT_TRUE(rb.push(1, data, 1));
    EXPECT_TRUE(rb.push(2, data, 1));
    EXPECT_TRUE(rb.push(3, data, 1));

    ASSERT_TRUE(rb.pop().has_value());
    ASSERT_TRUE(rb.pop().has_value());

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

template <typename RB>
void impl_reset()
{
    RB rb;

    uint8_t data[2] = {1, 2};

    EXPECT_TRUE(rb.push(10, data, 2));
    EXPECT_TRUE(rb.push(20, data, 2));

    rb.reset();

    EXPECT_FALSE(rb.pop().has_value());

    EXPECT_TRUE(rb.push(30, data, 2));

    auto v = rb.pop();

    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->event_id, 30);
    EXPECT_EQ(v->sz, 2);
}

template <typename RB>
void impl_dataPointerPerSlot()
{
    RB rb;

    uint8_t a[2] = {1, 2};
    uint8_t b[2] = {3, 4};

    EXPECT_TRUE(rb.push(1, a, 2));
    EXPECT_TRUE(rb.push(2, b, 2));

    auto v1 = rb.pop();
    auto v2 = rb.pop();

    ASSERT_TRUE(v1 && v2);

    EXPECT_NE(v1->data, v2->data);

    EXPECT_EQ(std::memcmp(v1->data, a, 2), 0);
    EXPECT_EQ(std::memcmp(v2->data, b, 2), 0);
}

//
// ------------------------------------------------------------
// Test wrappers
// ------------------------------------------------------------
//

#define GENERATE_TESTS(space)                                                                                          \
    TEST(space##_Tests, push)                                                                                          \
    {                                                                                                                  \
        impl_push<space>();                                                                                            \
    }                                                                                                                  \
                                                                                                                       \
    TEST(space##_Tests, pop)                                                                                           \
    {                                                                                                                  \
        impl_pop<space>();                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    TEST(space##_Tests, wrapAround)                                                                                    \
    {                                                                                                                  \
        impl_wrapAround<space>();                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    TEST(space##_Tests, reset)                                                                                         \
    {                                                                                                                  \
        impl_reset<space>();                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    TEST(space##_Tests, dataPointerPerSlot)                                                                            \
    {                                                                                                                  \
        impl_dataPointerPerSlot<space>();                                                                              \
    }

EVENT_RB_TYPES(GENERATE_TESTS)
