#pragma once

#include <iostream>

namespace psi::ipc {

template <size_t MAX_QUEUE_SIZE = 1024u, size_t MAX_DATA_LENGTH = 512u>
class CallbackSpace final
{
    using CallbackData = uint8_t[MAX_QUEUE_SIZE][1 + MAX_DATA_LENGTH];

    static constexpr size_t STATUS_INDEX = 0;

public:
    CallbackSpace()
        : m_cbData()
    {
    }

    void setAvailable(bool value)
    {
        m_isAvailable = value;
    }

    bool isAvailable() const
    {
        return m_isAvailable;
    }

    bool isCallbackAvailable(size_t cbIndex) const
    {
        return m_cbData[cbIndex - 1][STATUS_INDEX] == 0b11;
    }

    size_t pushCallback()
    {
        for (size_t i = 0; i < MAX_QUEUE_SIZE; ++i) {
            if (m_cbData[i][STATUS_INDEX] == 0b00) {
                m_cbData[i][STATUS_INDEX] = 0b01;
                return i + 1;
            }
        }

        std::cout << "[pushCallback] Not enough space for callback processing" << std::endl;
        return 0;
    }

    void updateCallback(size_t cbIndex, const uint8_t *data, const size_t sz)
    {
        if (cbIndex == 0) {
            std::cerr << "[updateCallback] Incorrect cbIndex" << std::endl;
            return;
        }

        if (m_cbData[cbIndex - 1][STATUS_INDEX] == 0b01) {
            memcpy(&m_cbData[cbIndex - 1][STATUS_INDEX + 1], data, sz);
            m_cbData[cbIndex - 1][STATUS_INDEX] = 0b11;
        }
    }

    uint8_t *popCallback(size_t cbIndex)
    {
        if (cbIndex == 0) {
            std::cerr << "[popCallback] Incorrect cbIndex" << std::endl;
            return nullptr;
        }

        if (m_cbData[cbIndex - 1][STATUS_INDEX] == 0b11) {
            uint8_t *data = new uint8_t[MAX_DATA_LENGTH]();
            memcpy(data, &m_cbData[cbIndex - 1][STATUS_INDEX] + 1, MAX_DATA_LENGTH);
            memset(&m_cbData[cbIndex - 1], 0, 1 + MAX_DATA_LENGTH);
            return data;
        }

        return nullptr;
    }

    void clearCallback(size_t cbIndex)
    {
        memset(&m_cbData[cbIndex - 1], 0, 1 + MAX_DATA_LENGTH);
    }

private:
    CallbackData m_cbData;
    bool m_isAvailable = false;
};

} // namespace psi::ipc
