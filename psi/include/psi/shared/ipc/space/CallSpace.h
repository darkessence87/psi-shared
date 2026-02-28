#pragma once

#include <array>
#include <iostream>
#include <optional>

namespace psi::ipc {

template <uint16_t MAX_QUEUE_SIZE = 1024u, uint16_t MAX_DATA_LENGTH = 512u>
class CallSpace final
{
    static constexpr size_t DATA_LENGTH = static_cast<size_t>(MAX_DATA_LENGTH) * MAX_QUEUE_SIZE;
    static_assert(DATA_LENGTH <= std::numeric_limits<uint32_t>::max(), "Index type too small for buffer");
    using Data = std::array<uint8_t, DATA_LENGTH>;

public:
    static constexpr uint16_t CALL_SZ = MAX_DATA_LENGTH;

    CallSpace()
        : m_data()
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

    bool isDataAvailable() const
    {
        return m_currentWriteIndex > 0;
    }

    bool push(uint8_t *data, const uint32_t sz) noexcept
    {
        if (!data || sz == 0) {
            return false;
        }

        if (sz > MAX_DATA_LENGTH) {
            return false;
        }

        if (m_currentWriteIndex + sz > DATA_LENGTH) {
            return false;
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
        std::memcpy(&m_data[m_currentWriteIndex], data, sz);
#pragma clang diagnostic pop
        m_currentWriteIndex += sz;

        return true;
    }

    const uint8_t *pop(uint32_t &dataSz) noexcept
    {
        dataSz = m_currentWriteIndex;
        return dataSz ? m_data.data() : nullptr;
    }

    void clear()
    {
        m_currentWriteIndex = 0;
    }

    std::optional<uint16_t> generateClientId()
    {
        if (m_lastClientId == std::numeric_limits<uint16_t>::max()) {
            std::cout << "Too many clients used" << std::endl;
            return std::nullopt;
        }
        return ++m_lastClientId;
    }

private:
    Data m_data;
    bool m_isAvailable = false;
    uint32_t m_currentWriteIndex = 0;
    uint16_t m_lastClientId = 0;
};

} // namespace psi::ipc
