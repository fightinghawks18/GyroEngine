//
// Created by lepag on 6/7/2025.
//

#include "printer.h"

void Printer::Log(const std::string& message)
{
    OnLogMessage(PrintType::Print, message);
}

void Printer::LogWarning(const std::string& message)
{
    OnLogMessage(PrintType::Warn, message);
}

void Printer::LogError(const std::string& message)
{
    OnLogMessage(PrintType::Error, message);
}

void Printer::OnLogMessage(PrintType printType, const std::string& message)
{
    std::cout << message << std::endl;
}
