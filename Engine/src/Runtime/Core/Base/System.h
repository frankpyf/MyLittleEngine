#pragma once

#include "Runtime/Events/Event.h"

namespace engine
{
    // @brief System Base Class
    class System
    {
    public:
        friend class EventBus;
        System(const std::string name);
        virtual ~System() = default;
        virtual void OnEvent(Event& event) = 0;
        virtual void Tick(float time_step) = 0;
    protected:
        std::string system_name_;
    private:
        System* next_;
    };
} // namespace engine
