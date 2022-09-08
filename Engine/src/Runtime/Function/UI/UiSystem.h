#pragma once
#include "Runtime/Core/Base/System.h"

namespace engine
{
    class UiSystem : public System
    {
    public:
        UiSystem();
        virtual void OnEvent(Event& event) override;
        virtual void OnUpdate(float timestep) override;
        void Init();
        void Shutdown();
    };
} // namespace engine
