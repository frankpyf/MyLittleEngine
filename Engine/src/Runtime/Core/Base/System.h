#pragma once

#include "Runtime/Events/Event.h"
#include "Runtime/Events/EventBus.h"

namespace engine
{
    // we may also want to make every system a singleton,
    // or we only make application a singleton, and every time
    // we want to call Notify, we do it through app_instance_(present solution)
    class System
    {
    public:
        friend class EventBus;
        System(const std::string name);
        virtual ~System() = default;
        virtual void OnEvent(Event& event) = 0;
        virtual void OnUpdate(float time_step) = 0;
    private:
        std::string system_name_;
        System* next_;
    };
} // namespace engine
