#pragma once
#include "Runtime/Core/Base/System.h"
namespace engine {
    class InputSystem :
        public System
    {
    private:
        void OnKeyEvent(Event& event);
    };
}

