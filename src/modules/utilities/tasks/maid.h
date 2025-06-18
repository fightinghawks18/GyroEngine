//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <functional>
#include <stack>

/// @brief Garbage collector
class Maid {
public:
    Maid() = default;
    ~Maid();

    void Add(const std::function<void()>& task);
    void Cleanup();
private:
    std::stack<std::function<void()>> m_tasks;
};
