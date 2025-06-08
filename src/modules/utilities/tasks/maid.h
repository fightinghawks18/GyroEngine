//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <vector>
#include <functional>

/// @brief Garbage collector
class Maid {
public:
    Maid() = default;
    ~Maid();

    void add(const std::function<void()>& task);
    void cleanup();
private:
    std::vector<std::function<void()>> m_tasks;
};
