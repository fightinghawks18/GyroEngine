//
// Created by lepag on 6/19/2025.
//

#pragma once

namespace GyroEngine::Utils
{

    template <typename T>
    class ISingleton
    {
    public:
        ISingleton(const ISingleton&) = delete;
        ISingleton& operator=(const ISingleton&) = delete;
        ISingleton(ISingleton&&) = delete;
        ISingleton& operator=(ISingleton&&) = delete;

        static T& Get() {
            static T instance;
            return instance;
        }
    protected:
        ISingleton() = default;
        ~ISingleton() = default;
    };

}
