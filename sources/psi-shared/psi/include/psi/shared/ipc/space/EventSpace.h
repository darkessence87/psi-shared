#pragma once

#include <bitset>
#include <optional>
#include <vector>

namespace psi::ipc {

template <size_t MAX_CLIENTS_SIZE = 16u>
class EventSpace final
{
    static constexpr size_t EVENT_MAX_ID = 32u;
    static constexpr size_t MAX_QUEUE_SIZE = 64u;
    static constexpr size_t MAX_DATA_LEN = 512u;
    using EventData = uint8_t[MAX_CLIENTS_SIZE][EVENT_MAX_ID][MAX_QUEUE_SIZE * MAX_DATA_LEN];

    static constexpr size_t EVENT_MAX_NAME = 64u;
    using EventNameData = uint8_t[EVENT_MAX_ID][EVENT_MAX_NAME];

public:
    void setAvailable(bool value)
    {
        m_isAvailable = value;

        if (!m_isAvailable) {
            resetData();
        }
    }

    bool isAvailable() const
    {
        return m_isAvailable;
    }

    std::optional<size_t> registerClient()
    {
        if (m_registeredClients.all()) {
            return std::nullopt;
        }

        for (size_t bit = 0; bit < MAX_CLIENTS_SIZE; ++bit) {
            if (!m_registeredClients.test(bit)) {
                m_registeredClients.set(bit);
                return bit;
            }
        }

        return std::nullopt;
    }

    void unregisterClient(size_t clientId)
    {
        if (clientId < MAX_CLIENTS_SIZE) {
            m_registeredClients.set(clientId, false);
        }
    }

    bool isDataAvailable(size_t clientId, size_t eventId) const
    {
        return m_isDataAvailable[clientId][eventId];
    }

    std::optional<size_t> eventId(const char *evName) const
    {
        for (size_t i = 0; i < EVENT_MAX_ID; ++i) {
            if (strcmp(evName, (const char *)(m_evNameData[i])) == 0) {
                return i;
            }
        }

        return std::nullopt;
    }

    std::optional<size_t> registerEvent(const char *evName)
    {
        std::optional<size_t> lastUsedIndex;

        // event is already registered
        for (size_t i = 0; i < EVENT_MAX_ID; ++i) {
            const auto str = (const char *)(m_evNameData[i]);
            if (strcmp(evName, str) == 0) {
                return i;
            } else if (strlen(str) > 0) {
                lastUsedIndex = i;
            }
        }

        // event is unknown
        if (!lastUsedIndex.has_value() || ((lastUsedIndex.value() + 1) < EVENT_MAX_ID)) {
            lastUsedIndex = lastUsedIndex.has_value() ? lastUsedIndex.value() + 1 : 0;
            memcpy(&m_evNameData[lastUsedIndex.value()], evName, strlen(evName));
            return lastUsedIndex;
        }

        // no space left for events
        std::cout << "Cannot register event: [" << evName << "], no space left" << std::endl;

        return std::nullopt;
    }

    void push(size_t eventId, uint8_t *data, const size_t sz)
    {
        const std::vector<size_t> clientIds = parseClientIds();
        for (size_t clientId : clientIds) {
            auto &wi = m_pushIndex[clientId][eventId];
            auto &ri = m_popIndex[clientId][eventId];
            auto &ei = m_endIndex[clientId][eventId];

            // std::cout << "push. [wi-ri-ei-sz]:[" << wi << "-" << ri << "-" << ei << "-" << sz << "]" << std::endl;

            // overflow case
            if (wi + sz > MAX_DATA_LEN * MAX_QUEUE_SIZE) {
                // std::cout << "Queue overflow!" << std::endl;
                ei = wi;
                wi = 0;
            }

            // override case
            if (wi + sz > ri) {
                // std::cout << "Queue override!" << std::endl;
                memset(&m_evData[clientId][eventId][ri], uint8_t(0), ei);
                ri = wi;
            }

            memcpy(&m_evData[clientId][eventId][wi], data, sz);
            wi += sz;
            m_isDataAvailable[clientId][eventId] = true;
        }
    }

    uint8_t *pop(size_t clientId, size_t eventId, size_t &dataLen)
    {
        auto &wi = m_pushIndex[clientId][eventId];
        auto &ri = m_popIndex[clientId][eventId];
        auto &ei = m_endIndex[clientId][eventId];

        uint8_t *q = nullptr;
        // overridden moment
        if ((wi == ri && ei > 0) || wi < ri) {
            dataLen = ei;
            q = new uint8_t[dataLen]();
            memcpy(q, &m_evData[clientId][eventId][ri], ei - ri);
            memcpy(&q[ei - ri], &m_evData[clientId][eventId][0], wi);
        // regular case
        } else if (wi > ri) {
            dataLen = wi;
            q = new uint8_t[dataLen]();
            memcpy(q, &m_evData[clientId][eventId][0], wi);
        }
        
        // std::cout << "pop. [wi-ri-ei-sz]:[" << wi << "-" << ri << "-" << ei << "-" << dataLen << "]" << std::endl;

        memset(&m_evData[clientId][eventId], uint8_t(0), MAX_QUEUE_SIZE * MAX_DATA_LEN);
        wi = 0;
        ri = 0;
        ei = 0;
        m_isDataAvailable[clientId][eventId] = false;
        return q;
    }

private:
    void resetData()
    {
        memset(&m_evData, 0, sizeof(m_evData));
        memset(&m_evNameData, 0, sizeof(m_evNameData));
        memset(&m_isDataAvailable, 0, sizeof(m_isDataAvailable));
        memset(&m_pushIndex, 0, sizeof(m_pushIndex));
    }

    std::vector<size_t> parseClientIds()
    {
        std::vector<size_t> result;

        for (size_t bit = 0; bit < MAX_CLIENTS_SIZE; ++bit) {
            if (m_registeredClients.test(bit)) {
                result.emplace_back(bit);
            }
        }

        return result;
    }

private:
    EventData m_evData = {};
    EventNameData m_evNameData = {};
    bool m_isAvailable = false;
    bool m_isDataAvailable[MAX_CLIENTS_SIZE][EVENT_MAX_ID] = {};
    size_t m_pushIndex[MAX_CLIENTS_SIZE][EVENT_MAX_ID] = {};
    size_t m_popIndex[MAX_CLIENTS_SIZE][EVENT_MAX_ID] = {};
    size_t m_endIndex[MAX_CLIENTS_SIZE][EVENT_MAX_ID] = {};
    std::bitset<MAX_CLIENTS_SIZE> m_registeredClients = 0;
};

} // namespace psi::ipc
