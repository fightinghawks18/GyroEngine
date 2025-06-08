//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <iostream>
#include <string>

class Printer {
public:
    static void print(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
private:
    enum class PrintType
    {
        Print,
        Warn,
        Error
    };
    static void out(PrintType printType, const std::string& message);
};
