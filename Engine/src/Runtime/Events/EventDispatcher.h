#pragma once
#include "Runtime/Core/Base/System.h"
#include "Event.h"

namespace engine
{
    // @brief Base Event Dispatcher
    class EventDispatcher
    {
    public:
        EventDispatcher();
        virtual ~EventDispatcher();
        virtual void Dispatch(Event& event);

        virtual void AddObserver(std::pair<std::string, std::function<void(const Event&)>> name_function_pair);
        virtual void RemoveObserver(std::string name);
        
    private:
        std::unordered_map<std::string, std::function<void(const Event&)>> observer_funcitons_;
    };
} // namespace engine
