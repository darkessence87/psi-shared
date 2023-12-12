#pragma once

#include <iostream>

//#include "common/Encryptor.h"

namespace psi::ipc {

template <size_t MAX_QUEUE_SIZE = 1024u, size_t MAX_DATA_LENGTH = 512u>
class CallSpace final
{
    static constexpr size_t DATA_LENGTH = MAX_DATA_LENGTH * MAX_QUEUE_SIZE;
    using Data = uint8_t[DATA_LENGTH];

public:
    CallSpace()
        : m_data()
    {
        //memset(&m_key, uint8_t(0), 32u);
    }

    /*CallSpace(const ByteBuffer &key)
        : m_data()
    {
        memcpy(key.data(), m_key, 32u);
    }*/

    void setAvailable(bool value)
    {
        m_isAvailable = value;
    }

    bool isAvailable() const
    {
        return m_isAvailable;
    }

    bool isDataAvailable() const
    {
        return m_isDataAvailable;
    }

    //void push(uint8_t *data, const size_t sz)
    //{
    //    const auto encoded = encode(data, sz);
    //    uint8_t *encodedData = encoded.data();
    //    size_t newSz = encoded.size() + 2;

    //    /// @todo Cyclic buffer should be used here
    //    if (m_currentWriteIndex + newSz >= DATA_LENGTH) {
    //        std::cout << "Override data, queue is full!" << std::endl;
    //        //return;
    //        m_currentWriteIndex = 0;
    //    }
    //    // std::cout << "[push] m_currentWriteIndex: " << m_currentWriteIndex << std::endl;

    //    const uint16_t checkSum = static_cast<uint16_t>(encoded.size());
    //    memcpy(&m_data[m_currentWriteIndex], &checkSum, 2);
    //    memcpy(&m_data[m_currentWriteIndex + 2], encodedData, encoded.size());
    //    m_currentWriteIndex += newSz;
    //    m_isDataAvailable = true;
    //}

    //uint8_t *pop(size_t &dataSz)
    //{
    //    // std::cout << "[pop] m_currentWriteIndex: " << m_currentWriteIndex << std::endl;
    //    uint8_t *q = new uint8_t[m_currentWriteIndex]();

    //    size_t w1 = 0;
    //    size_t w2 = 0;
    //    while (true) {
    //        uint16_t sz = 0;
    //        memcpy(&sz, &m_data[w1], 2);
    //        if (sz == 0) {
    //            break;
    //        }

    //        uint8_t *data = new uint8_t[sz];
    //        memcpy(data, &m_data[w1 + 2], sz);
    //        w1 += 2 + sz;
    //        
    //        const auto decoded = decode(data, sz);
    //        memcpy(&q[w2], decoded.data(), decoded.size());
    //        w2 += decoded.size();
    //    }

    //    dataSz = w2;

    //    memset(&m_data, uint8_t(0), DATA_LENGTH);
    //    m_currentWriteIndex = 0;
    //    m_isDataAvailable = false;
    //    return q;
    //}

    void push(uint8_t *data, const size_t sz)
    {
        /// @todo Cyclic buffer should be used here
        if (m_currentWriteIndex + sz >= DATA_LENGTH) {
            std::cout << "Override data, queue is full!" << std::endl;
            //return;
            m_currentWriteIndex = 0;
        }
        // std::cout << "[push] m_currentWriteIndex: " << m_currentWriteIndex << std::endl;

        memcpy(&m_data[m_currentWriteIndex], data, sz);
        m_currentWriteIndex += sz;
        m_isDataAvailable = true;
    }

    uint8_t *pop(size_t &dataSz)
    {
        // std::cout << "[pop] m_currentWriteIndex: " << m_currentWriteIndex << std::endl;
        if (!m_currentWriteIndex) {
            return nullptr;
        }
        dataSz = m_currentWriteIndex;
        uint8_t *q = new uint8_t[m_currentWriteIndex]();
        memcpy(q, m_data, m_currentWriteIndex);
        memset(&m_data, uint8_t(0), DATA_LENGTH);
        m_currentWriteIndex = 0;
        m_isDataAvailable = false;
        return q;
    }

    uint16_t generateClientId()
    {
        if ((m_lastClientId + 1) == 0) {
            std::cout << "Too many clients used" << std::endl;
            return 0;
        }
        return ++m_lastClientId;
    }

private:
    /*ByteBuffer encode(uint8_t *data, size_t sz)
    {
        ByteBuffer dataBuffer(data, sz);
        ByteBuffer keyBuffer(32u);
        keyBuffer.writeArray(m_key, 32u);
        return Encryptor::encryptAes256(dataBuffer, keyBuffer);
    }

    ByteBuffer decode(uint8_t *data, size_t sz)
    {
        ByteBuffer keyBuffer(32u);
        keyBuffer.writeArray(m_key, 32u);
        return Encryptor::decryptAes256(ByteBuffer(std::move(data), sz), keyBuffer);
    }*/

private:
    Data m_data;
    //uint8_t m_key[32];
    bool m_isAvailable = false;
    bool m_isDataAvailable = false;
    size_t m_currentWriteIndex = 0;
    uint16_t m_lastClientId = 0;
};

} // namespace psi::ipc
