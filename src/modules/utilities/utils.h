//
// Created by tyler on 6/12/25.
//

#pragma once
#include <filesystem>
#include <string>

#ifdef __linux__
#include <unistd.h>
#include <climits>
#elifdef _WIN32
#include <windows.h>
#endif

namespace utils
{
    static std::string getExecutableDir()
    {
#ifdef __linux__
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX - 1);
        if (count != -1) {
            result[count] = '\0';
            return std::filesystem::path(result).parent_path().string();
        }
        return {};
#elifdef _WIN32
        char result[MAX_PATH];
        GetModuleFileNameA(nullptr, result, MAX_PATH);
        return std::filesystem::path(result).parent_path().string();
#endif
    }
}
