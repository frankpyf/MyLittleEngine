#include "mlepch.h"

#include "Application.h"
#include "Runtime/Function/RHI/RHI.h"

// **************************************
// Adapted from Dear ImGui Vulkan example
// **************************************
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

extern bool g_ApplicationRunning;

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace engine {

	Application* Application::app_instance_ = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: app_specification_(specification)
	{
		//some assert(!app_instance_...) right here
		app_instance_ = this;
		app_window_ = std::make_unique<Window>(WindowProps(app_specification_.name, app_specification_.width, app_specification_.height));
		app_window_->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
		Init();
	}

	Application::~Application()
	{;
		Shutdown();
	}

	void Application::Init()
	{
		renderer_->Init();
		for (auto layer : layer_stack_)
			layer->OnAttach();
		//EventBus& bus = EventBus::GetInstance();
	}

	void Application::OnEvent(Event& event)
	{
#ifdef MLE_DEBUG
		MLE_CORE_INFO("Dispatching");
#endif // MLE_DEBUG
		/*EventBus& bus = EventBus::GetInstance();
		bus.Post(event);*/
	}

	void Application::Shutdown()
	{
		/*EventBus& bus = EventBus::GetInstance();
		bus.Close();*/

		for (auto& layer : layer_stack_)
			layer->OnDetach();

		layer_stack_.clear();

		// Cleanup
		renderer_->Shutdown();

		g_ApplicationRunning = false;
	}

	void Application::Run()
	{
		is_running_ = true;
		Application& app = Application::GetApp();
		GLFWwindow* glfw_window_handle = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		/*ImGuiIO& io = ImGui::GetIO();*/

		// Main loop
		while (!glfwWindowShouldClose(glfw_window_handle) && is_running_)
		{
			float delta_time;
			{
				using namespace std::chrono;

				steady_clock::time_point tick_time_point = steady_clock::now();
				duration<float> time_span = duration_cast<duration<float>>(tick_time_point - last_tick_time_point_);
				delta_time = time_span.count();

				last_tick_time_point_ = tick_time_point;
			}
			app_window_.get()->OnUpdate();

			if(!is_minimized_)
			{
				for (auto& layer : layer_stack_)
					layer->OnUpdate(delta_time);

				renderer_->Begin();
				for (auto& layer : layer_stack_)
					layer->OnUIRender();

				renderer_->Tick(delta_time);
				renderer_->End();
			}
			
		}
	}

	void Application::Close()
	{
		is_running_ = false;
	}

	void Application::OnWindowResize(WindowResizeEvent& event)
	{
		if (event.GetWidth() == 0 || event.GetHeight() == 0)
		{
			is_minimized_ = true;
			return;
		}

		is_minimized_ = false;
		//renderer_->Resize(event.GetWidth(), event.GetHeight());
	}
}
