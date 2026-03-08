
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

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<uint32_t>(data, offset);
    EXPECT_EQ(offset, uint16_t(4));
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

    uint16_t offset = 0;

    auto ra = deserializer::deserializeType<uint32_t>(data, offset);
    auto rb = deserializer::deserializeType<uint64_t>(data, offset);
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

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<std::string>(data, offset);
    EXPECT_EQ(offset, uint16_t(STRING_HEADER_SIZE + s.size()));
    EXPECT_EQ(out, s);
}

TEST(SerializeDeserialize_Tests, EmptyString)
{
    std::string s;

    uint8_t data[128] = {};
    auto sz = serializer::serializeType(data, s);

    ASSERT_EQ(sz, sizeof(uint64_t));

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<std::string>(data, offset);
    EXPECT_EQ(offset, STRING_HEADER_SIZE);
    EXPECT_TRUE(out.empty());
}

TEST(SerializeDeserialize_Tests, TuplePod)
{
    uint32_t a = 10;
    uint64_t b = 20;

    uint8_t data[128] = {};
    uint16_t sz = 0;

    serializer::serializeTuple(data, a, sz);
    serializer::serializeTuple(data, b, sz);

    uint16_t offset = 0;

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
    uint16_t sz = 0;

    serializer::serializeTuple(data, a, sz);
    serializer::serializeTuple(data, s, sz);
    serializer::serializeTuple(data, b, sz);

    uint16_t offset = 0;

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

TEST(SerializeDeserialize_Tests, VectorPod)
{
    std::vector<uint32_t> v = {1, 2, 3, 4};

    uint8_t data[256] {};
    uint16_t sz = serializer::serializeType(data, v);
    EXPECT_EQ(sz, uint16_t(24));

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<std::vector<uint32_t>>(data, offset);
    EXPECT_EQ(offset, uint16_t(VECTOR_HEADER_SIZE + v.size() * sizeof(uint32_t)));
    EXPECT_EQ(out.size(), v.size());

    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(out[i], v[i]);
    }
}

TEST(SerializeDeserialize_Tests, VectorEmpty)
{
    std::vector<uint32_t> v;

    uint8_t data[64] {};
    uint16_t sz = 0;

    serializer::serializeTuple(data, uint64_t(v.size()), sz);

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<std::vector<uint32_t>>(data, offset);
    EXPECT_EQ(offset, VECTOR_HEADER_SIZE);
    EXPECT_TRUE(out.empty());
}

TEST(SerializeDeserialize_Tests, VectorString)
{
    std::vector<std::string> v = {"a", "bb", "ccc"};

    uint8_t data[256] {};
    uint16_t sz = 0;

    serializer::serializeTuple(data, uint64_t(v.size()), sz);
    for (auto &e : v) {
        serializer::serializeTuple(data, e, sz);
    }

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<std::vector<std::string>>(data, offset);
    EXPECT_EQ(out.size(), v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(out[i], v[i]);
    }
}

TEST(SerializeDeserialize_Tests, TupleWithVector)
{
    uint32_t a = 10;
    std::vector<uint64_t> v = {5, 6, 7};

    uint8_t data[256] {};
    uint16_t sz = 0;

    serializer::serializeTuple(data, a, sz);
    serializer::serializeTuple(data, uint64_t(v.size()), sz);
    for (auto &e : v) {
        serializer::serializeTuple(data, e, sz);
    }

    uint16_t offset = 0;
    uint32_t ra;
    deserializer::deserializeTuple(ra, data, offset);
    auto rv = deserializer::deserializeType<std::vector<uint64_t>>(data, offset);
    EXPECT_EQ(ra, a);
    EXPECT_EQ(rv.size(), v.size());

    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(rv[i], v[i]);
    }
}

TEST(SerializeDeserialize_Tests, VariadicWithVector)
{
    uint32_t a = 3;
    std::vector<uint32_t> v = {8, 9};
    uint64_t b = 99;

    uint8_t data[256] {};
    uint16_t sz = 0;

    serializer::serializeTuple(data, a, sz);
    serializer::serializeTuple(data, uint64_t(v.size()), sz);
    for (auto &e : v) {
        serializer::serializeTuple(data, e, sz);
    }
    serializer::serializeTuple(data, b, sz);
    EXPECT_EQ(sz, size_t(28));

    auto t = deserializer::deserializeTypeVariadic<uint32_t, std::vector<uint32_t>, uint64_t>(
        data,
        0,
        static_cast<uint32_t *>(nullptr),
        static_cast<std::vector<uint32_t> *>(nullptr),
        static_cast<uint64_t *>(nullptr));

    EXPECT_EQ(std::get<0>(t), a);
    EXPECT_EQ(std::get<2>(t), b);

    auto &rv = std::get<1>(t);
    EXPECT_EQ(rv.size(), v.size());
    EXPECT_EQ(rv[0], v[0]);
    EXPECT_EQ(rv[1], v[1]);
}

TEST(SerializeDeserialize_Tests, VectorLarge)
{
    std::vector<uint64_t> v(100);

    for (size_t i = 0; i < v.size(); ++i) {
        v[i] = i * 3;
    }

    uint8_t data[4096] {};
    uint16_t sz = 0;

    serializer::serializeTuple(data, uint64_t(v.size()), sz);
    for (auto &e : v) {
        serializer::serializeTuple(data, e, sz);
    }

    uint16_t offset = 0;
    auto out = deserializer::deserializeType<std::vector<uint64_t>>(data, offset);
    EXPECT_EQ(out.size(), v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(out[i], v[i]);
    }
}
