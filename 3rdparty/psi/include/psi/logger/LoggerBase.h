#pragma once

#include <sstream>

namespace psi::logger {

inline const char *filename(const char *path)
{
    const char *file = path;
    for (; path && *path; ++path) {
        if (*path == '/' || *path == '\\') {
            file = path + 1;
        }
    }
    return file;
}

enum class LogLevel : uint8_t
{
    LVL_TRACE = 1,
    LVL_DEBUG,
    LVL_INFO,
    LVL_WARNING,
    LVL_ERROR
};
const char *asString(LogLevel) noexcept;

class ILoggerSingleton
{
public:
    virtual ~ILoggerSingleton() = default;

    virtual bool needLog(LogLevel) const = 0;
    virtual std::ostringstream &logStream(LogLevel, const char *, const char *, int, size_t) = 0;
    virtual std::ostringstream &logStreamShort() = 0;
    virtual void flush(bool /*isShort*/ = false) = 0;

    virtual void setLogLevel(LogLevel) = 0;
};

ILoggerSingleton &getLoggerInstance(const char *);

} // namespace psi::logger

#ifndef LOG_CTX
#define LOG_CTX "LOGG"
#endif

#define LOG_MSG(x, level)                                                                                              \
    do {                                                                                                               \
        auto &logger = psi::logger::getLoggerInstance(LOG_CTX);                                                        \
        if (!logger.needLog(level)) {                                                                                  \
            break;                                                                                                     \
        }                                                                                                              \
        const size_t _thisAddress_ = reinterpret_cast<size_t>(this);                                                   \
        auto &logStream =                                                                                              \
            logger.logStream(level, psi::logger::filename(__FILE__), __FUNCTION__, __LINE__, _thisAddress_);           \
        logStream << x;                                                                                                \
        logger.flush();                                                                                                \
    } while (0)

#define LOG_MSG_STATIC(x, level)                                                                                       \
    do {                                                                                                               \
        auto &logger = psi::logger::getLoggerInstance(LOG_CTX);                                                        \
        if (!logger.needLog(level)) {                                                                                  \
            break;                                                                                                     \
        }                                                                                                              \
        auto &logStream = logger.logStream(level, psi::logger::filename(__FILE__), __FUNCTION__, __LINE__, 0);         \
        logStream << x;                                                                                                \
        logger.flush();                                                                                                \
    } while (0)

#define LOG_MSG_SHORT(x)                                                                                               \
    do {                                                                                                               \
        auto &logger = psi::logger::getLoggerInstance(LOG_CTX);                                                        \
        auto &logStream = logger.logStreamShort();                                                                     \
        logStream << x;                                                                                                \
        logger.flush(true);                                                                                            \
    } while (0)
