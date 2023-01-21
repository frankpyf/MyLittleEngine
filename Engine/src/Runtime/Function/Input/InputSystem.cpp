#include "mlepch.h"
#include "InputSystem.h"
#include "Runtime/Core/Base/Application.h"
#include "GLFW/glfw3.h"

namespace engine {
	bool InputSystem::IsKeyDown(KeyCode keycode)
	{
		GLFWwindow* windowHandle = (GLFWwindow*)Application::GetApp().GetWindow().GetNativeWindow();
		int state = glfwGetKey(windowHandle, (int)keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool InputSystem::IsMouseButtonDown(MouseButton button)
	{
		GLFWwindow* windowHandle = (GLFWwindow*)Application::GetApp().GetWindow().GetNativeWindow();
		int state = glfwGetMouseButton(windowHandle, (int)button);
		return state == GLFW_PRESS;
	}

	glm::vec2 InputSystem::GetMousePosition()
	{
		GLFWwindow* windowHandle = (GLFWwindow*)Application::GetApp().GetWindow().GetNativeWindow();

		double x, y;
		glfwGetCursorPos(windowHandle, &x, &y);
		return { (float)x, (float)y };
	}

	void InputSystem::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* windowHandle = (GLFWwindow*)Application::GetApp().GetWindow().GetNativeWindow();
		glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}

	void InputSystem::ProcessInput()
	{
		std::for_each(imc_.begin(), imc_.end(), [=](InputMappingContext& imc) {

			});
	}
}