#pragma once

namespace engine {
    template<typename T>
    class Singleton
    {
    protected:
        Singleton() = default;

    public:
        static T& GetInstance() noexcept(std::is_nothrow_constructible<T>::value)
        {
            // C++11
            // instance will be initialized in the data area
            // and it will be initialized only once
            static T instance;
            return instance;
        }
        virtual ~Singleton() noexcept = default;
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
    };
}

