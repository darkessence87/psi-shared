
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/CallSpace.h"

using namespace psi::ipc;
using namespace psi::test;

using TestCallSpace = CallSpace<16, 32>; // DATA_LENGTH = 512

TEST(CallSpace_Tests, isAvailable)
{
    TestCallSpace cs;

    // default
    EXPECT_FALSE(cs.isAvailable());

    // set true
    cs.setAvailable(true);
    EXPECT_TRUE(cs.isAvailable());

    // set false
    cs.setAvailable(false);
    EXPECT_FALSE(cs.isAvailable());
}

TEST(CallSpace_Tests, isDataAvailable)
{
    TestCallSpace cs;

    EXPECT_FALSE(cs.isDataAvailable());

    uint8_t data[3] = {1, 2, 3};
    cs.push(data, 3);
    EXPECT_TRUE(cs.isDataAvailable());

    uint32_t sz = 0;
    uint8_t *out = cs.pop(sz);

    EXPECT_NE(out, nullptr);
    EXPECT_EQ(sz, 3);

    EXPECT_TRUE(cs.isDataAvailable());

    cs.clear();
    EXPECT_FALSE(cs.isDataAvailable());
}

TEST(CallSpace_Tests, push)
{
    TestCallSpace cs;

    // single push
    {
        uint8_t data[4] = {1, 2, 3, 4};
        cs.push(data, 4);

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 4);
        EXPECT_EQ(std::memcmp(out, data, 4), 0);

        cs.clear();
    }

    // multiple push concatenation
    {
        uint8_t a[2] = {5, 6};
        uint8_t b[3] = {7, 8, 9};

        cs.push(a, 2);
        cs.push(b, 3);

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 5);

        uint8_t expected[5] = {5, 6, 7, 8, 9};
        EXPECT_EQ(std::memcmp(out, expected, 5), 0);

        cs.clear();
    }

    // override when full
    {
        std::vector<uint8_t> big(500, 1);
        std::vector<uint8_t> small(20, 2);

        cs.push(big.data(), static_cast<uint32_t>(big.size()));
        cs.push(small.data(), static_cast<uint32_t>(small.size())); // overflow → reset

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, small.size());
        EXPECT_EQ(std::memcmp(out, small.data(), small.size()), 0);

        cs.clear();
    }
}

TEST(CallSpace_Tests, pop)
{
    TestCallSpace cs;

    // pop on empty
    {
        uint32_t sz = 123;
        EXPECT_EQ(cs.pop(sz), nullptr);
        EXPECT_EQ(sz, 0u);
    }

    // pop returns correct data
    {
        uint8_t data[3] = {9, 8, 7};
        cs.push(data, 3);

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 3);
        EXPECT_EQ(std::memcmp(out, data, 3), 0);

        cs.clear();
    }

    // repeated pop without clear returns same pointer and size
    {
        uint8_t data[2] = {1, 1};
        cs.push(data, 2);

        uint32_t sz1 = 0;
        uint8_t *p1 = cs.pop(sz1);

        uint32_t sz2 = 0;
        uint8_t *p2 = cs.pop(sz2);

        EXPECT_EQ(p1, p2);
        EXPECT_EQ(sz1, sz2);

        cs.clear();
    }

    // after clear buffer is empty
    {
        uint8_t data[2] = {3, 3};
        cs.push(data, 2);

        uint32_t sz = 0;
        cs.pop(sz);
        cs.clear();

        EXPECT_EQ(cs.pop(sz), nullptr);
        EXPECT_FALSE(cs.isDataAvailable());
    }
}

TEST(CallSpace_Tests, generateClientId)
{
    TestCallSpace cs;

    EXPECT_EQ(cs.generateClientId(), 1);
    EXPECT_EQ(cs.generateClientId(), 2);
    EXPECT_EQ(cs.generateClientId(), 3);

    uint16_t last = 0;
    for (uint32_t i = 0; i < 65535 - 3; ++i) {
        last = cs.generateClientId();
    }

    EXPECT_EQ(last, 65535);
    EXPECT_EQ(cs.generateClientId(), 0);
}

TEST(CallSpace_Tests, CALL_SZ)
{
    EXPECT_EQ(TestCallSpace::CALL_SZ, 32);
}

TEST(CallSpace_Tests, FillBufferBoundary)
{
    using SmallCS = CallSpace<4, 8>; // DATA_LENGTH = 32
    SmallCS cs;

    // 1. exactly DATA_LENGTH
    {
        uint8_t data[32];
        std::memset(data, 1, sizeof(data));

        cs.push(data, 32);

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 32);
        EXPECT_EQ(std::memcmp(out, data, 32), 0);

        cs.clear();
    }

    // 2. DATA_LENGTH - 1
    {
        uint8_t data[31];
        std::memset(data, 2, sizeof(data));

        cs.push(data, 31);

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 31);
        EXPECT_EQ(std::memcmp(out, data, 31), 0);

        cs.clear();
    }

    // 3. overflow → reset
    {
        uint8_t data1[30];
        uint8_t data2[4];

        std::memset(data1, 3, sizeof(data1));
        std::memset(data2, 4, sizeof(data2));

        cs.push(data1, 30);
        cs.push(data2, 4); // 30 + 4 > 32 → reset

        uint32_t sz = 0;
        uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 4);
        EXPECT_EQ(std::memcmp(out, data2, 4), 0);

        cs.clear();
    }
}
