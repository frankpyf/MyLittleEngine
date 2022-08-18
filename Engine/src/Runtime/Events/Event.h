#pragma once
namespace engine
{
	// Both enum classes are form Hazel Engine
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		ApplicationEvent = 1 << 0,
		InputEvent = 1 << 1,
		KeyboardEvent = 1 << 2,
		MouseEvent = 1 << 3,
		MouseButtonEvent = 1 << 4
	};

    class Event
    {
	public:
        virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual int GetCategoryFlags() const = 0;

		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
    };
} // namespace engine
