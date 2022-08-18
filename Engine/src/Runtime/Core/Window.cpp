#include "mlepch.h"

#include "Window.h"
#include <GLFW/glfw3.h>

#include "Runtime/Core/Log.h"
#include "Runtime/Events/ApplicationEvents.h"

namespace engine {
	static void GLFWErrorCallback(int error, const char* description)
	{
		MLE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	Window::Window(const WindowProps& props)
		:window_properties_(props)
	{
		Init(props);
	}

	void Window::Init(const WindowProps& props)
	{
		// Setup GLFW window
		glfwSetErrorCallback(GLFWErrorCallback);
		if (!glfwInit())
			return;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfw_window_ = glfwCreateWindow(props.width, props.height, props.title.c_str(), NULL, NULL);

		glfwSetWindowUserPointer(glfw_window_, &window_properties_);

		// set GLFW callback
		glfwSetWindowSizeCallback(glfw_window_, [](GLFWwindow* window, int width, int height)
			{
				WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);
				data.width = width;
				data.height = height;

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
				MLE_INFO("window resize event: {0},{1}", width, height);
			});

		glfwSetKeyCallback(glfw_window_, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					/*KeyPressedEvent event(key, 0);
					data.EventCallback(event);*/
					MLE_INFO("key pressed: {0}", key);
					break;
				}
				case GLFW_RELEASE:
				{
					/*KeyReleasedEvent event(key);
					data.EventCallback(event);*/
					MLE_INFO("key released: {0}", key);
					break;
				}
				case GLFW_REPEAT:
				{
					/*KeyPressedEvent event(key, true);
					data.EventCallback(event);*/
					MLE_INFO("key repeat: {0}", key);
					break;
				}
				}
			});

		glfwSetMouseButtonCallback(glfw_window_, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					/*MouseButtonPressedEvent event(button);
					data.EventCallback(event);*/
					MLE_INFO("mouse pressed: {0}", button);
					break;
				}
				case GLFW_RELEASE:
				{
					/*MouseButtonReleasedEvent event(button);
					data.EventCallback(event);*/
					MLE_INFO("mouse released: {0}", button);
					break;
				}
				}
			});
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Shutdown()
	{
		//MLE_CORE_INFO("Engine is shutting down")
		glfwDestroyWindow(glfw_window_);
		glfwTerminate();
	}
}