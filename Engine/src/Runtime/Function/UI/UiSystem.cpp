#include "mlepch.h"
#include "UiSystem.h"
#include "Runtime/Core/Base/Log.h"
#include "Runtime/Events/Event.h"

#include "Runtime/Core/Base/Application.h" 

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

namespace engine {
	UiSystem::UiSystem()
		: System("UiSystem")
	{
		Init();
	}

	void UiSystem::OnEvent(Event& event)
	{
		switch (event.GetEventType()) {
		case EventType::WindowResize:
			MLE_CORE_INFO("{0} recieved {1} event", system_name_, event.GetCategoryFlags());
		}
		
	}

	void UiSystem::OnUpdate(float timestep)
	{

	}

	void UiSystem::Init()
	{
		//Application& app = Application::GetApp();
		//GLFWwindow* glfw_window_handle = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		//// Setup Dear ImGui context
		//IMGUI_CHECKVERSION();
		//ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		////io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		////io.ConfigViewportsNoAutoMerge = true;
		////io.ConfigViewportsNoTaskBarIcon = true;

		//// Setup Dear ImGui style
		//ImGui::StyleColorsDark();
		////ImGui::StyleColorsClassic();

		//// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		//ImGuiStyle& style = ImGui::GetStyle();
		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	style.WindowRounding = 0.0f;
		//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		//}

		//// Setup Platform/Renderer backends
		//ImGui_ImplGlfw_InitForVulkan(glfw_window_handle, true);
		//ImGui_ImplVulkan_InitInfo init_info = {};
		//init_info.Instance = g_Instance;
		//init_info.PhysicalDevice = g_PhysicalDevice;
		//init_info.Device = g_Device;
		//init_info.QueueFamily = g_QueueFamily;
		//init_info.Queue = g_Queue;
		//init_info.PipelineCache = g_PipelineCache;
		//init_info.DescriptorPool = g_DescriptorPool;
		//init_info.Subpass = 0;
		//init_info.MinImageCount = g_MinImageCount;
		//init_info.ImageCount = wd->ImageCount;
		//init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		//init_info.Allocator = g_Allocator;
		//init_info.CheckVkResultFn = check_vk_result;
		//ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

		//// Load default font
		//ImFontConfig fontConfig;
		//fontConfig.FontDataOwnedByAtlas = false;
		//ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
		//io.FontDefault = robotoFont;

		//// Upload Fonts
		//{
		//	// Use any command queue
		//	VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
		//	VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

		//	err = vkResetCommandPool(g_Device, command_pool, 0);
		//	check_vk_result(err);
		//	VkCommandBufferBeginInfo begin_info = {};
		//	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		//	err = vkBeginCommandBuffer(command_buffer, &begin_info);
		//	check_vk_result(err);

		//	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		//	VkSubmitInfo end_info = {};
		//	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//	end_info.commandBufferCount = 1;
		//	end_info.pCommandBuffers = &command_buffer;
		//	err = vkEndCommandBuffer(command_buffer);
		//	check_vk_result(err);
		//	err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
		//	check_vk_result(err);

		//	err = vkDeviceWaitIdle(g_Device);
		//	check_vk_result(err);
		//	ImGui_ImplVulkan_DestroyFontUploadObjects();
		//}
	}
}