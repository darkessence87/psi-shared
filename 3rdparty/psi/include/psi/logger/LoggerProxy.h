#pragma once

#include "generated_LoggerClient.h"

namespace psi::logger {

class LoggerProxy : public LoggerClient
{
public:
    LoggerProxy();

    void log(uint16_t arg0, std::string arg1) override;
};

} // namespace psi::logger