#pragma once
#include "Runtime/Core/Base/System.h"
#include "Event.h"

namespace engine
{
    /*struct SystemNode {
        SystemNode* next;
        System& system_reference;
    };*/

    // I don't want to create a event bus instance every time i dispatch an event,
    // so i made it a singleton. It will be initialized during the engine initialization.
    class EventBus
    {
    public:
        EventBus();
        ~EventBus();
        static EventBus& GetInstance();
        void Dispatch(Event& event);

        void AddSubscriber(System* node);
        void RemoveSubscriber(System* node);

        void Close();
    private:
        // Event Bus Instance
        static EventBus* instance_;
        // subscriber link list
        System* head_;
        //SystemNode* app_head_;
    };
} // namespace engine
