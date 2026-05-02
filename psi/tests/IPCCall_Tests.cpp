
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/IPCCall.h"

using namespace psi::ipc;
using namespace psi::test;

TEST(IPCCall_Tests, IPCCall_serialize)
{
    constexpr uint64_t kSize = 6 + sizeof(uint64_t) + sizeof(uint16_t);

    uint8_t buffer[64];

    // case 1: zero values
    {
        memset(buffer, 0xcd, sizeof(buffer));

        IPCCall call {};
        uint64_t written = call.serialize(buffer);

        EXPECT_EQ(written, kSize);

        uint64_t callId {};
        uint16_t method {};
        uint16_t client {};
        uint16_t cbIndex {};
        uint16_t callSz {};

        memcpy(&callId, buffer, 8);
        memcpy(&method, buffer + 8, 2);
        memcpy(&client, buffer + 10, 2);
        memcpy(&cbIndex, buffer + 12, 2);
        memcpy(&callSz, buffer + 14, 2);

        EXPECT_EQ(callId, 0);
        EXPECT_EQ(method, 0);
        EXPECT_EQ(client, 0);
        EXPECT_EQ(cbIndex, 0);
        EXPECT_EQ(callSz, 0);

        for (uint64_t i = written; i < sizeof(buffer); ++i) {
            EXPECT_EQ(buffer[i], 0xcd);
        }
    }

    // case 2: normal values
    {
        memset(buffer, 0, sizeof(buffer));

        IPCCall call {0x11223344, 0x5566, 0x7788, 0x99aa, 0xbbcc};
        call.serialize(buffer);

        uint64_t callId {};
        uint16_t method {};
        uint16_t client {};
        uint16_t cbIndex {};
        uint16_t callSz {};

        memcpy(&callId, buffer, 8);
        memcpy(&method, buffer + 8, 2);
        memcpy(&client, buffer + 10, 2);
        memcpy(&cbIndex, buffer + 12, 2);
        memcpy(&callSz, buffer + 14, 2);

        EXPECT_EQ(callId, call.m_call_id);
        EXPECT_EQ(method, call.m_method_id);
        EXPECT_EQ(client, call.m_client_id);
        EXPECT_EQ(cbIndex, call.m_cb_index);
        EXPECT_EQ(callSz, call.m_args_sz);
    }

    // case 3: max values
    {
        memset(buffer, 0, sizeof(buffer));

        IPCCall call {std::numeric_limits<uint64_t>::max(),
                      std::numeric_limits<uint16_t>::max(),
                      std::numeric_limits<uint16_t>::max(),
                      std::numeric_limits<uint16_t>::max(),
                      std::numeric_limits<uint16_t>::max()};

        call.serialize(buffer);

        uint64_t callId {};
        uint16_t method {};
        uint16_t client {};
        uint16_t cbIndex {};
        uint16_t callSz {};

        memcpy(&callId, buffer, 8);
        memcpy(&method, buffer + 8, 2);
        memcpy(&client, buffer + 10, 2);
        memcpy(&cbIndex, buffer + 12, 2);
        memcpy(&callSz, buffer + 14, 2);

        EXPECT_EQ(callId, call.m_call_id);
        EXPECT_EQ(method, call.m_method_id);
        EXPECT_EQ(client, call.m_client_id);
        EXPECT_EQ(cbIndex, call.m_cb_index);
        EXPECT_EQ(callSz, call.m_args_sz);
    }
}

TEST(IPCCall_Tests, IPCCall_deserialize)
{
    uint8_t buffer[128];

    auto writeRaw = [&](uint64_t callId, uint16_t method, uint16_t client, uint16_t cbIndex, uint16_t callSz, size_t offset) {
        memcpy(buffer + offset + 0, &callId, 8);
        memcpy(buffer + offset + 8, &method, 2);
        memcpy(buffer + offset + 10, &client, 2);
        memcpy(buffer + offset + 12, &cbIndex, 2);
        memcpy(buffer + offset + 14, &callSz, 2);
    };

    // case 1
    {
        memset(buffer, 0, sizeof(buffer));

        writeRaw(1, 2, 3, 4, 5, 0);

        IPCCall result;
        result.deserialize(buffer, 0);

        EXPECT_EQ(result.m_call_id, 1);
        EXPECT_EQ(result.m_method_id, 2);
        EXPECT_EQ(result.m_client_id, 3);
        EXPECT_EQ(result.m_cb_index, 4);
        EXPECT_EQ(result.m_args_sz, 5);
    }

    // case 2: offset
    {
        memset(buffer, 0, sizeof(buffer));

        writeRaw(10, 20, 30, 40, 50, 13);

        IPCCall result;
        result.deserialize(buffer, 13);

        EXPECT_EQ(result.m_call_id, 10);
        EXPECT_EQ(result.m_method_id, 20);
        EXPECT_EQ(result.m_client_id, 30);
        EXPECT_EQ(result.m_cb_index, 40);
        EXPECT_EQ(result.m_args_sz, 50);
    }

    // case 3: boundary - args_sz exactly at MAX_ARGS_SZ is preserved
    {
        memset(buffer, 0, sizeof(buffer));

        writeRaw(std::numeric_limits<uint64_t>::max(),
                 std::numeric_limits<uint16_t>::max(),
                 std::numeric_limits<uint16_t>::max(),
                 std::numeric_limits<uint16_t>::max(),
                 IPCCall::MAX_ARGS_SZ,
                 5);

        IPCCall result;
        result.deserialize(buffer, 5);

        EXPECT_EQ(result.m_call_id, std::numeric_limits<uint64_t>::max());
        EXPECT_EQ(result.m_method_id, std::numeric_limits<uint16_t>::max());
        EXPECT_EQ(result.m_client_id, std::numeric_limits<uint16_t>::max());
        EXPECT_EQ(result.m_cb_index, std::numeric_limits<uint16_t>::max());
        EXPECT_EQ(result.m_args_sz, IPCCall::MAX_ARGS_SZ);
    }

    // case 4: oversized args_sz is clamped to 0
    {
        memset(buffer, 0, sizeof(buffer));

        uint16_t oversized = IPCCall::MAX_ARGS_SZ + 1u;
        writeRaw(1, 2, 3, 4, oversized, 0);

        IPCCall result;
        result.deserialize(buffer, 0);

        EXPECT_EQ(result.m_call_id, 1);
        EXPECT_EQ(result.m_method_id, 2);
        EXPECT_EQ(result.m_client_id, 3);
        EXPECT_EQ(result.m_cb_index, 4);
        EXPECT_EQ(result.m_args_sz, 0u);
    }
}

TEST(IPCCall_Tests, IPCCall_roundtrip)
{
    uint8_t buffer[32];

    IPCCall src {0x5566778899aabbccull, 0x1122, 0x3344, 0xddee, 0x01f0};
    uint64_t written = src.serialize(buffer);

    IPCCall dst;
    dst.deserialize(buffer, 0);

    EXPECT_EQ(written, 6 + sizeof(uint64_t) + sizeof(uint16_t));
    EXPECT_EQ(dst.m_method_id, src.m_method_id);
    EXPECT_EQ(dst.m_client_id, src.m_client_id);
    EXPECT_EQ(dst.m_call_id, src.m_call_id);
    EXPECT_EQ(dst.m_cb_index, src.m_cb_index);
    EXPECT_EQ(dst.m_args_sz, src.m_args_sz);
}

TEST(IPCCall_Tests, IPCCall_deserialize_does_not_touch_outside)
{
    uint8_t buffer[64];
    memset(buffer, 0xcd, sizeof(buffer));

    uint64_t offset = 10;
    uint16_t m = 1, c = 2, cb = 4, sz = 5;
    uint64_t id = 3;

    memcpy(buffer + offset + 0, &id, 8);
    memcpy(buffer + offset + 8, &m, 2);
    memcpy(buffer + offset + 10, &c, 2);
    memcpy(buffer + offset + 12, &cb, 2);
    memcpy(buffer + offset + 14, &sz, 2);

    IPCCall dst;
    dst.deserialize(buffer, offset);

    for (size_t i = 0; i < offset; ++i) {
        EXPECT_EQ(buffer[i], 0xcd);
    }
}

TEST(IPCCall_Tests, IPCCall_stream_layout)
{
    uint8_t buffer[64];

    IPCCall a {1, 2, 3, 4, 5};
    IPCCall b {5, 6, 7, 8, 9};

    uint64_t off = 0;
    off += a.serialize(buffer + off);
    off += b.serialize(buffer + off);

    IPCCall ra, rb;
    uint64_t readOff = 0;
    ra.deserialize(buffer, readOff);
    readOff += 4 + 8 + 2 + 2;
    rb.deserialize(buffer, readOff);

    EXPECT_EQ(ra.m_method_id, a.m_method_id);
    EXPECT_EQ(rb.m_method_id, b.m_method_id);
}

TEST(IPCCall_Tests, IPCCall_head_size_contract)
{
    static_assert(sizeof(uint16_t) == 2);
    static_assert(sizeof(uint64_t) == 8);
    static_assert(IPCCall::HEAD_SZ == 14);

    uint8_t buffer[64];
    memset(buffer, 0, sizeof(buffer));

    IPCCall call {1, 2, 3, 4, 5};
    uint64_t written = call.serialize(buffer);

    EXPECT_EQ(written, IPCCall::HEAD_SZ + 2u);

    EXPECT_EQ(buffer[IPCCall::HEAD_SZ + 2u], 0);
}

TEST(IPCCall_Tests, IPCCall_little_endian_layout)
{
    uint8_t buffer[IPCCall::HEAD_SZ + 2u];
    memset(buffer, 0, sizeof(buffer));

    IPCCall call {0x5566778899aabbccull, 0xddee, 0x1122, 0x3344, 0xff00};
    call.serialize(buffer);

    // callId = 0x5566778899aabbcc → cc bb aa 99 88 77 66 55
    EXPECT_EQ(buffer[0], 0xcc);
    EXPECT_EQ(buffer[1], 0xbb);
    EXPECT_EQ(buffer[2], 0xaa);
    EXPECT_EQ(buffer[3], 0x99);
    EXPECT_EQ(buffer[4], 0x88);
    EXPECT_EQ(buffer[5], 0x77);
    EXPECT_EQ(buffer[6], 0x66);
    EXPECT_EQ(buffer[7], 0x55);

    // methodId = 0xddee → ee dd
    EXPECT_EQ(buffer[8], 0xee);
    EXPECT_EQ(buffer[9], 0xdd);

    // clientId = 0x1122 → 22 11
    EXPECT_EQ(buffer[10], 0x22);
    EXPECT_EQ(buffer[11], 0x11);

    // cbIndex = 0x3344 → 44 33
    EXPECT_EQ(buffer[12], 0x44);
    EXPECT_EQ(buffer[13], 0x33);

    // callSz = 0xff00 → 00 ff
    EXPECT_EQ(buffer[14], 0x00);
    EXPECT_EQ(buffer[15], 0xff);
}

TEST(IPCCall_Tests, performance)
{
    constexpr int N = 5'000'000;
    constexpr size_t kSize = 4 + sizeof(size_t) + sizeof(uint16_t);

    alignas(64) uint8_t buffer[kSize * 64] {};

    IPCCall call_in {1, 2, 3, 4};
    IPCCall call_out;

    // ---------------- serialize ----------------
    {
        for (int i = 0; i < 10'000; ++i) {
            call_in.serialize(buffer);
        }

        TestHelper::timeFn_nano(
            "serialize",
            [&]() {
                call_in.serialize(buffer);
                asm volatile("" ::: "memory");
            },
            N);
    }

    // ---------------- deserialize ----------------
    {
        call_in.serialize(buffer);

        for (int i = 0; i < 10'000; ++i) {
            call_out.deserialize(buffer, 0);
        }

        TestHelper::timeFn_nano(
            "deserialize",
            [&]() {
                call_out.deserialize(buffer, 0);
                asm volatile("" ::: "memory");
            },
            N);
    }

    // ---------------- roundtrip ----------------
    {
        for (int i = 0; i < 10'000; ++i) {
            call_in.serialize(buffer);
            call_out.deserialize(buffer, 0);
            asm volatile("" ::: "memory");
        }

        TestHelper::timeFn_nano(
            "roundtrip",
            [&]() {
                call_in.serialize(buffer);
                call_out.deserialize(buffer, 0);
                asm volatile("" ::: "memory");
            },
            N);
    }

    // ---------------- stream serialize ----------------
    {
        size_t offset = 0;

        for (int i = 0; i < 10'000; ++i) {
            call_in.serialize(buffer + offset);
            offset += kSize;
            if (offset + kSize >= sizeof(buffer))
                offset = 0;
        }

        offset = 0;

        TestHelper::timeFn_nano(
            "serialize_stream",
            [&]() {
                call_in.serialize(buffer + offset);
                offset += kSize;
                if (offset + kSize >= sizeof(buffer))
                    offset = 0;
                asm volatile("" ::: "memory");
            },
            N);
    }
}
