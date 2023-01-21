#pragma once
#include "KeyCode.h"
#include "Runtime/Core/Base/System.h"
#include "Runtime/Core/Base/Singleton.h"
namespace engine {
    struct InputComponent;

    enum class TriggerEvent : uint8_t
    {

    };
    // some static functions
    class InputModifier
    {

    };
    struct InputAction 
    {
        TriggerEvent trigger_event;
        InputModifier* modifiers;

    };
    struct ActionEventBinding
    {

    };
    struct ActionKeyMapping
    {

    };
    struct InputMappingContext
    {
        std::vector<ActionKeyMapping> mappings;
    };


    class InputSystem : public Singleton<InputSystem>
    {
    public:
        void OnTick(InputComponent* input_components, const float detla_time);

        static bool IsKeyDown(KeyCode keycode);
        static bool IsMouseButtonDown(MouseButton button);
        static glm::vec2 GetMousePosition();

        static void SetCursorMode(CursorMode mode);
    private:
        void OnKeyEvent(Event& event);

        void ProcessInput();

        std::vector<InputMappingContext> imc_;

    };
}

