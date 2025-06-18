//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <string>

class Logger {
public:
    static void Log(const std::string& message);
    static void LogWarning(const std::string& message);
    static void LogError(const std::string& message);
private:
    enum class PrintType
    {
        Print,
        Warn,
        Error
    };
    static void OnLogMessage(PrintType printType, const std::string& message);
};
