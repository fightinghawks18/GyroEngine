//
// Created by lepag on 6/7/2025.
//

#include "maid.h"

Maid::~Maid()
{
    cleanup();
}

void Maid::add(const std::function<void()>& task)
{
    m_tasks.push_back(task);
}

void Maid::cleanup()
{
    for (const auto& task : m_tasks)
    {
        task();
    }
    m_tasks.clear();
}
