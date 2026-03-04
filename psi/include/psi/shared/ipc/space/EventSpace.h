#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <optional>

#include "EventRingBuffer.h"

namespace psi::ipc {

template <uint16_t MAX_CLIENTS_SIZE = 16u>
class EventSpace final
{
    static constexpr uint16_t MAX_QUEUE_SIZE = 128u;
    static constexpr uint16_t MAX_DATA_LENGTH = 256u;
    using EventData = std::array<EventRingBuffer<MAX_QUEUE_SIZE, MAX_DATA_LENGTH>, MAX_CLIENTS_SIZE>;

public:
    static constexpr uint16_t EVENT_SZ = MAX_DATA_LENGTH;

    struct EventView {
        uint16_t event_id;
        const uint8_t *data;
        uint16_t sz;
    };

    void setAvailable(bool value)
    {
        m_isAvailable = value;
    }

    bool isAvailable() const
    {
        return m_isAvailable;
    }

    std::optional<uint16_t> registerClient()
    {
        if (m_registeredClients.all()) {
            return std::nullopt;
        }

        for (uint16_t bit = 0; bit < MAX_CLIENTS_SIZE; ++bit) {
            if (!m_registeredClients.test(bit)) {
                m_registeredClients.set(bit);
                m_clients[bit].reset();
                return bit;
            }
        }

        return std::nullopt;
    }

    void unregisterClient(uint16_t clientId)
    {
        if (clientId < MAX_CLIENTS_SIZE) {
            m_registeredClients.set(clientId, false);
            m_clients[clientId].reset();
        }
    }

    bool push(uint16_t event_id, const uint8_t *data, uint16_t sz) noexcept
    {
        if (!m_isAvailable) {
            return false;
        }

        bool any = false;

        for (uint16_t clientId = 0; clientId < MAX_CLIENTS_SIZE; ++clientId) {
            if (!m_registeredClients.test(clientId)) {
                continue;
            }

            any |= m_clients[clientId].push(event_id, data, sz);
        }

        return any;
    }

    std::optional<EventView> pop(uint16_t client_id) noexcept
    {
        if (client_id >= MAX_CLIENTS_SIZE)
            return std::nullopt;

        if (!m_registeredClients.test(client_id))
            return std::nullopt;

        auto view = m_clients[client_id].pop();
        if (!view)
            return std::nullopt;

        return EventView {view->event_id, view->data, view->sz};
    }

private:
    EventData m_clients;
    std::bitset<MAX_CLIENTS_SIZE> m_registeredClients {};

    bool m_isAvailable = false;
};

} // namespace psi::ipc
