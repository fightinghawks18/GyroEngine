//
// Created by lepag on 6/7/2025.
//

#include "printer.h"

void Printer::print(const std::string& message)
{
    out(PrintType::Print, message);
}

void Printer::warn(const std::string& message)
{
    out(PrintType::Warn, message);
}

void Printer::error(const std::string& message)
{
    out(PrintType::Error, message);
}

void Printer::out(PrintType printType, const std::string& message)
{
    std::cout << message << std::endl;
}
