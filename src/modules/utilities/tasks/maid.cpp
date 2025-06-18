//
// Created by lepag on 6/7/2025.
//

#include "maid.h"

Maid::~Maid()
{
    Cleanup();
}

void Maid::Add(const std::function<void()>& task)
{
    m_tasks.push(task);
}

void Maid::Cleanup()
{
    while (!m_tasks.empty()) {
        m_tasks.top()();
        m_tasks.pop();
    }
}
