#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/space/CallSpace.h"

using namespace psi::ipc;
using namespace psi::test;

using TestCallSpace = CallSpace<16, 32>; // DATA_LENGTH = 512

TEST(CallSpace_Tests, isAvailable)
{
    TestCallSpace cs;

    EXPECT_FALSE(cs.isAvailable());

    cs.setAvailable(true);
    EXPECT_TRUE(cs.isAvailable());

    cs.setAvailable(false);
    EXPECT_FALSE(cs.isAvailable());
}

TEST(CallSpace_Tests, isDataAvailable)
{
    TestCallSpace cs;

    EXPECT_FALSE(cs.isDataAvailable());

    uint8_t data[3] = {1, 2, 3};
    EXPECT_TRUE(cs.push(data, 3));
    EXPECT_TRUE(cs.isDataAvailable());

    uint32_t sz = 0;
    const uint8_t *out = cs.pop(sz);

    EXPECT_NE(out, nullptr);
    EXPECT_EQ(sz, 3);
    EXPECT_TRUE(cs.isDataAvailable()); // pop does not consume

    cs.clear();
    EXPECT_FALSE(cs.isDataAvailable());
}

TEST(CallSpace_Tests, push)
{
    TestCallSpace cs;

    // single push
    {
        uint8_t data[4] = {1, 2, 3, 4};
        EXPECT_TRUE(cs.push(data, 4));

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 4);
        EXPECT_EQ(std::memcmp(out, data, 4), 0);

        cs.clear();
    }

    // multiple push concatenation (each chunk <= CALL_SZ)
    {
        uint8_t a[16];
        uint8_t b[16];
        std::memset(a, 5, sizeof(a));
        std::memset(b, 6, sizeof(b));

        EXPECT_TRUE(cs.push(a, 16));
        EXPECT_TRUE(cs.push(b, 16));

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 32);

        std::vector<uint8_t> expected(32);
        std::memset(expected.data(), 5, 16);
        std::memset(expected.data() + 16, 6, 16);

        EXPECT_EQ(std::memcmp(out, expected.data(), 32), 0);

        cs.clear();
    }

    // fill buffer to DATA_LENGTH using valid chunks
    {
        std::vector<uint8_t> chunk(TestCallSpace::CALL_SZ, 1);

        for (int i = 0; i < 16; ++i) {
            EXPECT_TRUE(cs.push(chunk.data(), chunk.size()));
        }

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 16 * TestCallSpace::CALL_SZ); // 512
        EXPECT_EQ(out[0], 1);

        cs.clear();
    }

    // overflow → push fails, buffer unchanged
    {
        std::vector<uint8_t> chunk(TestCallSpace::CALL_SZ, 2);

        for (int i = 0; i < 16; ++i) {
            EXPECT_TRUE(cs.push(chunk.data(), chunk.size()));
        }

        EXPECT_FALSE(cs.push(chunk.data(), chunk.size())); // no space left

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 16 * TestCallSpace::CALL_SZ);
        EXPECT_EQ(out[0], 2);

        cs.clear();
    }

    // reject > MAX_DATA_LENGTH
    {
        std::vector<uint8_t> tooBig(TestCallSpace::CALL_SZ + 1, 1);
        EXPECT_FALSE(cs.push(tooBig.data(), static_cast<uint32_t>(tooBig.size())));
        EXPECT_FALSE(cs.isDataAvailable());
    }

    // reject null / zero
    {
        uint8_t dummy = 0;
        EXPECT_FALSE(cs.push(nullptr, 10));
        EXPECT_FALSE(cs.push(&dummy, 0));
        EXPECT_FALSE(cs.isDataAvailable());
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
        EXPECT_TRUE(cs.push(data, 3));

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 3);
        EXPECT_EQ(std::memcmp(out, data, 3), 0);

        cs.clear();
    }

    // repeated pop without clear returns same pointer and size
    {
        uint8_t data[2] = {1, 1};
        EXPECT_TRUE(cs.push(data, 2));

        uint32_t sz1 = 0;
        const uint8_t *p1 = cs.pop(sz1);

        uint32_t sz2 = 0;
        const uint8_t *p2 = cs.pop(sz2);

        EXPECT_EQ(p1, p2);
        EXPECT_EQ(sz1, sz2);

        cs.clear();
    }

    // after clear buffer is empty
    {
        uint8_t data[2] = {3, 3};
        EXPECT_TRUE(cs.push(data, 2));

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

    EXPECT_EQ(cs.generateClientId().value(), 1);
    EXPECT_EQ(cs.generateClientId().value(), 2);
    EXPECT_EQ(cs.generateClientId().value(), 3);

    uint16_t last = 0;
    for (uint32_t i = 0; i < 65535 - 3; ++i) {
        last = cs.generateClientId().value();
    }

    EXPECT_EQ(last, 65535);
    EXPECT_FALSE(cs.generateClientId().has_value());
}

TEST(CallSpace_Tests, CALL_SZ)
{
    EXPECT_EQ(TestCallSpace::CALL_SZ, 32);
}

TEST(CallSpace_Tests, FillBufferBoundary)
{
    using SmallCS = CallSpace<4, 8>; // DATA_LENGTH = 32, CALL_SZ = 8
    SmallCS cs;

    // exactly DATA_LENGTH (4 pushes × 8 bytes)
    {
        uint8_t chunk[8];
        std::memset(chunk, 1, sizeof(chunk));

        for (int i = 0; i < 4; ++i) {
            EXPECT_TRUE(cs.push(chunk, sizeof(chunk)));
        }

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 32);
        EXPECT_EQ(out[0], 1);
        EXPECT_EQ(out[31], 1);

        cs.clear();
    }

    // DATA_LENGTH - 1 (3×8 + 7)
    {
        uint8_t chunk8[8];
        uint8_t chunk7[7];
        std::memset(chunk8, 2, sizeof(chunk8));
        std::memset(chunk7, 2, sizeof(chunk7));

        for (int i = 0; i < 3; ++i) {
            EXPECT_TRUE(cs.push(chunk8, sizeof(chunk8)));
        }
        EXPECT_TRUE(cs.push(chunk7, sizeof(chunk7)));

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 31);
        EXPECT_EQ(out[0], 2);
        EXPECT_EQ(out[30], 2);

        cs.clear();
    }

    // overflow → push fails, buffer unchanged
    {
        uint8_t chunk8[8];
        std::memset(chunk8, 3, sizeof(chunk8));

        for (int i = 0; i < 4; ++i) {
            EXPECT_TRUE(cs.push(chunk8, sizeof(chunk8)));
        }

        EXPECT_FALSE(cs.push(chunk8, sizeof(chunk8))); // no space left

        uint32_t sz = 0;
        const uint8_t *out = cs.pop(sz);

        EXPECT_NE(out, nullptr);
        EXPECT_EQ(sz, 32);
        EXPECT_EQ(out[0], 3);

        cs.clear();
    }
}
