#include "mlepch.h"

#include "Application.h"
#include "Log.h"
#include "Runtime/Events/EventBus.h"
#include "Runtime/Platform/Vulkan/VulkanRHI.h"
#include "Runtime/Platform/Vulkan/VulkanDevice.h"

// **************************************
// Adapted from Dear ImGui Vulkan example
// **************************************
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


// Emedded font
#include "Runtime/ImGui/Roboto-Regular.embed"

extern bool g_ApplicationRunning;
extern bool g_SwapChainRebuild;

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
		Application& app = Application::GetApp();
		GLFWwindow* glfw_window_handle	= static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		renderer_ = new Renderer();
		renderer_->Init();

		EventBus& bus = EventBus::GetInstance();
		/*ui_system_ = new UiSystem();
		bus.AddSubscriber(ui_system_);*/
	}

	void Application::OnEvent(Event& event)
	{
#ifdef MLE_DEBUG
		MLE_CORE_INFO("Dispatching");
#endif // MLE_DEBUG
		EventBus& bus = EventBus::GetInstance();
		bus.Dispatch(event);
	}

	void Application::Shutdown()
	{
		EventBus& bus = EventBus::GetInstance();
		bus.Close();

		for (auto& layer : layer_stack_)
			layer->OnDetach();

		layer_stack_.clear();

		// Cleanup
		VkResult err = vkDeviceWaitIdle(renderer_->GetRHI()->GetDevice()->GetDeviceHandle());
		check_vk_result(err);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		//ImGui::DestroyContext();

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
			// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			app_window_.get()->OnUpdate();

			//// Start the Dear ImGui frame
			//ImGui_ImplVulkan_NewFrame();
			//ImGui_ImplGlfw_NewFrame();
			//ImGui::NewFrame();

			//{
			//	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			//	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			//	// because it would be confusing to have two docking targets within each others.
			//	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
			//	if (menu_bar_callback_)
			//		window_flags |= ImGuiWindowFlags_MenuBar;

			//	const ImGuiViewport* viewport = ImGui::GetMainViewport();
			//	ImGui::SetNextWindowPos(viewport->WorkPos);
			//	ImGui::SetNextWindowSize(viewport->WorkSize);
			//	ImGui::SetNextWindowViewport(viewport->ID);
			//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			//	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			//	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			//	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			//	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
			//	// and handle the pass-thru hole, so we ask Begin() to not render a background.
			//	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			//		window_flags |= ImGuiWindowFlags_NoBackground;

			//	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			//	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			//	// all active windows docked into it will lose their parent and become undocked.
			//	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			//	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			//	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			//	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
			//	ImGui::PopStyleVar();

			//	ImGui::PopStyleVar(2);

			//	// Submit the DockSpace
			//	ImGuiIO& io = ImGui::GetIO();
			//	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			//	{
			//		ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
			//		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			//	}

			//	if (menu_bar_callback_)
			//	{
			//		if (ImGui::BeginMenuBar())
			//		{
			//			menu_bar_callback_();
			//			ImGui::EndMenuBar();
			//		}
			//	}

			//	for (auto& layer : layer_stack_)
			//		layer->OnUIRender();

			//	ImGui::End();
			//}

			//// Rendering
			//ImGui::Render();
			//ImDrawData* main_draw_data = ImGui::GetDrawData();
			//const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
			//wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
			//wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
			//wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
			//wd->ClearValue.color.float32[3] = clear_color.w;
			//if (!main_is_minimized)
			//	FrameRender(wd, main_draw_data);

			//// Update and Render additional Platform Windows
			//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			//{
			//	ImGui::UpdatePlatformWindows();
			//	ImGui::RenderPlatformWindowsDefault();
			//}

			//// Present Main Platform Window
			//if (!main_is_minimized)
			//	FramePresent(wd);
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
		renderer_->Resize(event.GetWidth(), event.GetHeight());
	}
}
