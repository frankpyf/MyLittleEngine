#pragma once

#include "Event.h"

namespace engine {
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: width_(width), height_(height) {}

		unsigned int GetWidth() const { return width_; }
		unsigned int GetHeight() const { return height_; }

		virtual EventType GetEventType()	const override { return EventType::WindowResize; };
		virtual int GetCategoryFlags()		const override { return ApplicationEvent; };
	private:
		unsigned int width_, height_;
	};
}