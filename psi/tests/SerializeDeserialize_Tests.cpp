
#include "psi/test/TestHelper.h"
#include "psi/test/psi_mock.h"

#include "psi/shared/ipc/protocol/Deserializer.h"
#include "psi/shared/ipc/protocol/Serializer.h"

using namespace psi::ipc;
using namespace psi::test;

TEST(SerializeDeserialize_Tests, PodSingle)
{
    uint32_t v = 0xdeadbeef;

    uint8_t data[128] = {};
    auto sz = serializer::serializeType(data, v);
    EXPECT_EQ(sz, sizeof(uint32_t));

    auto out = deserializer::deserializeType<uint32_t>(data, 0);
    EXPECT_EQ(out, v);
}

TEST(SerializeDeserialize_Tests, PodMultiple)
{
    uint32_t a = 1;
    uint64_t b = 2;
    uint16_t c = 3;

    uint8_t data[128] = {};
    auto sz = serializer::serializeType(data, a, b, c);
    EXPECT_EQ(sz, sizeof(a) + sizeof(b) + sizeof(c));

    uint64_t offset = 0;

    auto ra = deserializer::deserializeType<uint32_t>(data, offset);
    offset += sizeof(uint32_t);

    auto rb = deserializer::deserializeType<uint64_t>(data, offset);
    offset += sizeof(uint64_t);

    auto rc = deserializer::deserializeType<uint16_t>(data, offset);

    EXPECT_EQ(ra, a);
    EXPECT_EQ(rb, b);
    EXPECT_EQ(rc, c);
}

TEST(SerializeDeserialize_Tests, StringSingle)
{
    std::string s = "hello ipc";

    uint8_t data[256] = {};
    auto sz = serializer::serializeType(data, s);

    EXPECT_EQ(sz, sizeof(uint64_t) + s.size());

    auto out = deserializer::deserializeType<std::string>(data, 0);
    EXPECT_EQ(out, s);
}

TEST(SerializeDeserialize_Tests, EmptyString)
{
    std::string s;

    uint8_t data[128] = {};
    auto sz = serializer::serializeType(data, s);

    ASSERT_EQ(sz, sizeof(uint64_t));

    auto out = deserializer::deserializeType<std::string>(data, 0);
    EXPECT_TRUE(out.empty());
}

TEST(SerializeDeserialize_Tests, TuplePod)
{
    uint32_t a = 10;
    uint64_t b = 20;

    uint8_t data[128] = {};
    size_t sz = 0;

    serializer::serializeTuple(data, a, sz);
    serializer::serializeTuple(data, b, sz);

    uint64_t offset = 0;

    uint32_t ra;
    uint64_t rb;

    deserializer::deserializeTuple(ra, data, offset);
    deserializer::deserializeTuple(rb, data, offset);

    EXPECT_EQ(ra, a);
    EXPECT_EQ(rb, b);
}

TEST(SerializeDeserialize_Tests, TupleWithString)
{
    uint32_t a = 42;
    std::string s = "abc";
    uint64_t b = 999;

    uint8_t data[256] = {};
    size_t sz = 0;

    serializer::serializeTuple(data, a, sz);
    serializer::serializeTuple(data, s, sz);
    serializer::serializeTuple(data, b, sz);

    uint64_t offset = 0;

    uint32_t ra;
    std::string rs;
    uint64_t rb;

    deserializer::deserializeTuple(ra, data, offset);
    deserializer::deserializeTuple(rs, data, offset);
    deserializer::deserializeTuple(rb, data, offset);

    EXPECT_EQ(ra, a);
    EXPECT_EQ(rs, s);
    EXPECT_EQ(rb, b);
}

TEST(SerializeDeserialize_Tests, VariadicWithString)
{
    uint32_t a = 7;
    std::string s = "variadic";
    uint64_t b = 1234;

    uint8_t data[256] = {};
    serializer::serializeType(data, a, s, b);

    auto t = deserializer::deserializeTypeVariadic<uint32_t, std::string, uint64_t>(data,
                                                                                    0,
                                                                                    static_cast<uint32_t *>(nullptr),
                                                                                    static_cast<std::string *>(nullptr),
                                                                                    static_cast<uint64_t *>(nullptr));

    EXPECT_EQ(std::get<0>(t), a);
    EXPECT_EQ(std::get<1>(t), s);
    EXPECT_EQ(std::get<2>(t), b);
}

TEST(SerializeDeserialize_Tests, SizeOfArgsMatchesSerialize)
{
    uint32_t a = 1;
    std::string s = "xyz";
    uint64_t b = 2;

    uint8_t data[256] {};

    auto sz1 = serializer::serializeType(data, a, s, b);
    auto sz2 = serializer::sizeOfArgs(a, s, b);

    EXPECT_EQ(sz1, sz2);
}
