//
// Created by lepag on 6/7/2025.
//

#include "logger.h"

#include <iostream>

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

void Logger::OnLogMessage(PrintType printType, const std::string& message)
{
    std::cout << message << std::endl;
}
