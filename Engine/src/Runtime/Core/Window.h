#pragma once
#include "Runtime/Events/Event.h"
struct GLFWwindow;

namespace engine {

	struct WindowProps
	{
		std::string title;
		uint32_t width;
		uint32_t height;
		std::function<void(Event&)> EventCallback;

		WindowProps(const std::string& title = "My Little Engine",
			uint32_t width = 1600,
			uint32_t height = 900)
			: title(title), width(width), height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		Window(const WindowProps& props);
		virtual ~Window();

		void OnUpdate();

		uint32_t		GetWidth()			const	{ return window_properties_.width; };
		uint32_t		GetHeight()			const	{ return window_properties_.height; };
		virtual void*	GetNativeWindow()	const	{ return glfw_window_; };

		void SetEventCallback(const std::function<void(Event&)>& callback) { window_properties_.EventCallback = callback; };

	private:
		//data
		GLFWwindow* glfw_window_;
		WindowProps window_properties_;
		//function
		void Init(const WindowProps& props);
		void Shutdown();
	};
}

