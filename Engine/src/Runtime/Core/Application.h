#pragma once

#include "Runtime/Core/Layer.h"
#include "Runtime/Core/Window.h"

#include "imgui.h"
#include "vulkan/vulkan.h"

void check_vk_result(VkResult err);

struct GLFWwindow;

namespace engine {

	struct ApplicationSpecification
	{
		std::string name = "Walnut App";
		uint32_t width = 1600;
		uint32_t height = 900;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();

		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback) { menu_bar_callback_ = menubarCallback; }

		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			layer_stack_.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { layer_stack_.emplace_back(layer); layer->OnAttach(); }

		void Close();

		static Application& GetApp() { return *app_instance_; }
		Window& GetWindow() { return*app_window_; }


		static VkInstance GetInstance();
		static VkPhysicalDevice GetPhysicalDevice();
		static VkDevice GetDevice();

		static VkCommandBuffer GetCommandBuffer(bool begin);
		static void FlushCommandBuffer(VkCommandBuffer commandBuffer);

		static void SubmitResourceFree(std::function<void()>&& func);
	private:
		void Init();
		void Shutdown();
	private:
		static Application* app_instance_;
		ApplicationSpecification app_specification_;
		std::unique_ptr<Window> app_window_;
		bool is_running_ = false;

		std::vector<std::shared_ptr<Layer>> layer_stack_;
		std::function<void()> menu_bar_callback_;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}