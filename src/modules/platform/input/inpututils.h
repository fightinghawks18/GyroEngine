//
// Created by tyler on 7/3/25.
//

#pragma once

#include <stdexcept>
#include <variant>
#include <glm/vec2.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

namespace GyroEngine::Input
{
    using InputValue = std::variant<bool, int, float, double, glm::vec2>;

    template <typename T>
    T ReadInputValue(InputValue value)
    {
        if (std::holds_alternative<bool>(value))
        {
            if constexpr (std::is_same_v<T, int>)
            {
                return std::get<int>(value) == 1;
            } if constexpr (std::is_same_v<T, float>)
            {
                return std::get<float>(value) == 1;
            } else if constexpr (std::is_same_v<T, double>)
            {
                return std::get<double>(value) == 1;
            } else if constexpr (std::is_same_v<T, glm::vec2>)
            {
                throw std::runtime_error("Attempting to convert from bool to glm::vec2 is not allowed.");
            } else
            {
                return std::get<bool>(value);
            }
        }
        throw std::runtime_error("Unsupported type conversion for InputData value.");
    }
}