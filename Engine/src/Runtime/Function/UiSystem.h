#include "Runtime/Core/Base/System.h"

namespace engine
{
    class UiSystem : public System
    {
    public:
        virtual void OnEvent(Event& event) override;
    };
} // namespace engine
