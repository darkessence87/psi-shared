#pragma once

#include "LoggerBase.h"

#define LOG_DEBUG(x) LOG_MSG(x, psi::logger::LogLevel::LVL_DEBUG)

#define LOG_ERROR(x) LOG_MSG(x, psi::logger::LogLevel::LVL_ERROR)

#define LOG_INFO(x) LOG_MSG(x, psi::logger::LogLevel::LVL_INFO)

#define LOG_WARN(x) LOG_MSG(x, psi::logger::LogLevel::LVL_WARNING)

#define LOG_TRACE(x) LOG_MSG(x, psi::logger::LogLevel::LVL_TRACE)

#define LOG_DEBUG_STATIC(x) LOG_MSG_STATIC(x, psi::logger::LogLevel::LVL_DEBUG)

#define LOG_ERROR_STATIC(x) LOG_MSG_STATIC(x, psi::logger::LogLevel::LVL_ERROR)

#define LOG_INFO_STATIC(x) LOG_MSG_STATIC(x, psi::logger::LogLevel::LVL_INFO)

#define LOG_WARN_STATIC(x) LOG_MSG_STATIC(x, psi::logger::LogLevel::LVL_WARNING)

#define LOG_TRACE_STATIC(x) LOG_MSG_STATIC(x, psi::logger::LogLevel::LVL_TRACE)
