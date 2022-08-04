#include "mlepch.h"

#include "Window.h"
#include <GLFW/glfw3.h>

namespace Engine {
	static void GLFWErrorCallback(int error, const char* description)
	{
		//MLE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	Window::Window(const WindowProps& props)
		:m_Props(props)
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
		m_Window = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
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
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}
}