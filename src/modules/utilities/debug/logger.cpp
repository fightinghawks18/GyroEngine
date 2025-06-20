#include "logger.h"
#include <fmt/format.h>

void Logger::Log(const std::string& message)
{
    OnLogMessage(PrintType::Print, message);
}

void Logger::LogWarning(const std::string& message)
{
    OnLogMessage(PrintType::Warn, message);
}

void Logger::LogError(const std::string& message)
{
    OnLogMessage(PrintType::Error, message);
}

void Logger::OnLogMessage(PrintType /*printType*/, const std::string& message)
{
    fmt::print("{}\n", message);
}