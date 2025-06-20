#pragma once

#include <string>
#include <fmt/format.h>

class Logger {
public:
    static void Log(const std::string& message);
    static void LogWarning(const std::string& message);
    static void LogError(const std::string& message);

    template <typename... Args>
    static void Log(const std::string& formatStr, Args&&... args) {
        Log(fmt::format(formatStr, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void LogWarning(const std::string& formatStr, Args&&... args) {
        LogWarning(fmt::format(formatStr, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void LogError(const std::string& formatStr, Args&&... args) {
        LogError(fmt::format(formatStr, std::forward<Args>(args)...));
    }
private:
    enum class PrintType
    {
        Print,
        Warn,
        Error
    };
    static void OnLogMessage(PrintType printType, const std::string& message);
};