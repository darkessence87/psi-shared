#pragma once

#include <array>
#include <optional>

namespace psi::ipc {

template <uint16_t MAX_QUEUE_SIZE = 1024u, uint16_t MAX_DATA_LENGTH = 512u>
class CallbackSpace final
{
    static constexpr uint16_t STATUS_SLOT_SIZE = 1u;
    static constexpr uint16_t SIZE_SLOT_SIZE = sizeof(uint16_t);
    static constexpr uint16_t CALLBACK_SIZE = STATUS_SLOT_SIZE + SIZE_SLOT_SIZE + MAX_DATA_LENGTH;
    using CallbackData = std::array<std::array<uint8_t, CALLBACK_SIZE>, MAX_QUEUE_SIZE>;

    static constexpr uint16_t STATUS_INDEX = 0;
    static constexpr uint16_t SIZE_INDEX = 1;
    static constexpr uint16_t DATA_INDEX = 1 + SIZE_SLOT_SIZE;

    enum class DataState : uint8_t
    {
        NoData = 0b00,
        Pending = 0b01,
        Ready = 0b11,
    };

public:
    static constexpr uint16_t QUEUE_SIZE = MAX_QUEUE_SIZE;
    static constexpr uint16_t DATA_SIZE = MAX_DATA_LENGTH;

    struct CallbackView {
        uint8_t *data;
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

    bool isCallbackAvailable(uint16_t cbIndex) const
    {
        if (cbIndex >= MAX_QUEUE_SIZE) {
            return false;
        }

        return m_cbData[cbIndex][STATUS_INDEX] == uint8_t(DataState::Ready);
    }

    std::optional<uint16_t> pushCallback() noexcept
    {
        for (uint16_t i = 0; i < MAX_QUEUE_SIZE; ++i) {
            if (m_cbData[i][STATUS_INDEX] == uint8_t(DataState::NoData)) {
                m_cbData[i][STATUS_INDEX] = uint8_t(DataState::Pending);
                return i;
            }
        }

        return std::nullopt;
    }

    void updateCallback(uint16_t cbIndex, const uint8_t *data, const uint16_t sz) noexcept
    {
        if (cbIndex >= MAX_QUEUE_SIZE || sz > MAX_DATA_LENGTH) {
            return;
        }

        auto &cb = m_cbData[cbIndex];
        if (cb[STATUS_INDEX] != uint8_t(DataState::Pending)) {
            return;
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        std::memcpy(cb.data() + SIZE_INDEX, &sz, sizeof(sz));
        std::memcpy(cb.data() + DATA_INDEX, data, sz);
#pragma clang diagnostic pop

        cb[STATUS_INDEX] = uint8_t(DataState::Ready);
    }

    std::optional<CallbackView> popCallback(uint16_t cbIndex) noexcept
    {
        if (cbIndex >= MAX_QUEUE_SIZE) {
            return std::nullopt;
        }

        auto &cb = m_cbData[cbIndex];

        if (cb[STATUS_INDEX] != uint8_t(DataState::Ready)) {
            return std::nullopt;
        }

        cb[STATUS_INDEX] = uint8_t(DataState::NoData);
        uint16_t cb_data_size;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
        std::memcpy(&cb_data_size, cb.data() + SIZE_INDEX, SIZE_SLOT_SIZE);

        return CallbackView {cb.data() + DATA_INDEX, cb_data_size};
#pragma clang diagnostic pop
    }

    void clearCallback(uint16_t cbIndex) noexcept
    {
        if (cbIndex >= MAX_QUEUE_SIZE) {
            return;
        }

        m_cbData[cbIndex].fill(0);
    }

private:
    CallbackData m_cbData {};
    bool m_isAvailable = false;
};

using CallbackSpace_Default = CallbackSpace<>;
using CallbackSpace_Q_1024_D_1024 = CallbackSpace<1024, 1024>;
using CallbackSpace_Q_1024_D_2048 = CallbackSpace<1024, 2048>;
using CallbackSpace_Q_2048_D_512 = CallbackSpace<2048, 512>;
using CallbackSpace_Q_2048_D_1024 = CallbackSpace<2048, 1024>;
using CallbackSpace_Q_2048_D_2048 = CallbackSpace<2048, 2048>;

} // namespace psi::ipc
