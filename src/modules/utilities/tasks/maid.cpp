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
    m_tasks.push(task);
}

void Maid::cleanup()
{
    while (!m_tasks.empty()) {
        m_tasks.top()();
        m_tasks.pop();
    }
}
