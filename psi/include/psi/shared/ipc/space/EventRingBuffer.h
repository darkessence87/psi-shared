
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>

namespace psi::ipc {

template <uint16_t MAX_QUEUE_SIZE, uint16_t MAX_DATA_LENGTH>
class EventRingBuffer
{
    static constexpr uint16_t HEADER = sizeof(uint16_t) * 2;
    static constexpr uint16_t SLOT_SIZE = HEADER + MAX_DATA_LENGTH;

    using Storage = std::array<std::array<uint8_t, SLOT_SIZE>, MAX_QUEUE_SIZE>;

public:
    struct BufferView {
        uint16_t event_id;
        const uint8_t *data;
        uint16_t sz;
    };

    bool push(uint16_t event_id, const uint8_t *data, uint16_t sz) noexcept
    {
        if (sz > MAX_DATA_LENGTH) {
            return false;
        }

        uint16_t next = (m_write + 1) % MAX_QUEUE_SIZE;
        if (next == m_read) {
            return false;
        }

        auto &slot = m_storage[m_write];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        std::memcpy(slot.data(), &event_id, sizeof(uint16_t));
        std::memcpy(slot.data() + sizeof(uint16_t), &sz, sizeof(uint16_t));
        std::memcpy(slot.data() + HEADER, data, sz);
#pragma clang diagnostic pop

        m_write = next;
        return true;
    }

    std::optional<BufferView> pop() noexcept
    {
        if (m_read == m_write) {
            return std::nullopt;
        }

        auto &slot = m_storage[m_read];

        uint16_t event_id;
        uint16_t sz;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        std::memcpy(&event_id, slot.data(), sizeof(uint16_t));
        std::memcpy(&sz, slot.data() + sizeof(uint16_t), sizeof(uint16_t));

        BufferView v {event_id, slot.data() + HEADER, sz};
#pragma clang diagnostic pop

        m_read = (m_read + 1) % MAX_QUEUE_SIZE;
        return v;
    }

    void reset()
    {
        m_read = 0;
        m_write = 0;
    }

private:
    Storage m_storage {};
    uint16_t m_read = 0;
    uint16_t m_write = 0;
};

} // namespace psi::ipc
