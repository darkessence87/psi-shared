
#pragma once

#include <array>
#include <atomic>
#include <string>

namespace psi::logger {

template <size_t MAX_THREAD_NAME_LEN = 5u>
class IThreadMap
{
    class ThreadName
    {
    public:
        ThreadName();
        void setData(const char in[MAX_THREAD_NAME_LEN]);
        std::string getData() const;

    private:
        char data[MAX_THREAD_NAME_LEN];
    };

    using ThreadInfo = std::pair<uint16_t, ThreadName>;

public:
    IThreadMap();
    std::string currentThreadName();

private:
    std::string insertThread(uint16_t threadId);

private:
    std::atomic<int> m_threadLastMappedId;
    std::array<ThreadInfo, 5000> m_data;
};

} // namespace psi::logger
