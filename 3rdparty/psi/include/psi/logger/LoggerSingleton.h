#pragma once

#include <mutex>
#include <queue>

#include "IThreadMap.hpp"
#include "psi/comm/Subscription.h"

#include "LoggerBase.h"

namespace psi::logger {

class LoggerProxy;

class LoggerSingleton final : public ILoggerSingleton
{
public:
    static const char RECORD_SEPARATOR;
    static const size_t MAX_MSG_QUEUE_SZ;

    LoggerSingleton(const char *);
    ~LoggerSingleton();

    bool needLog(LogLevel) const override;
    std::ostringstream &logStream(LogLevel, const char *, const char *, int, size_t) override;
    std::ostringstream &logStreamShort() override;
    void flush(bool = false);

    void setLogLevel(LogLevel) override;

private:
    void onLoggerServiceStatusUpdate(bool);

    std::string currTimeLocal() const;
    std::string currThreadName();
    void generateHeader(LogLevel);
    void logTo(const std::string &);
    void sendToServer(const std::string &);

private:
    std::mutex m_mtx;
    IThreadMap<> m_threadMap;
    size_t m_timestampId = 0;
    std::shared_ptr<std::ostringstream> m_logStream;
    const std::string m_ctx;
    LogLevel m_level = LogLevel::LVL_TRACE;

    std::unique_ptr<LoggerProxy> m_loggerProxy;
    uint16_t m_clientId = 0;
    comm::Subscription m_loggerServiceAvailability;
    std::queue<std::string> m_msgQueue;
};

ILoggerSingleton &getLoggerInstance(const char *ctx)
{
    static LoggerSingleton mainLogger(ctx);
    return mainLogger;
}

} // namespace psi::logger
