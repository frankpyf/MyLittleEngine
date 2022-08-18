#include "mlepch.h"
#include "VulkanRHI.h"
#include <vector>
#include <GLFW/glfw3.h>
#include "Runtime/Core/Log.h"
//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif
void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	MLE_CORE_ERROR("[vulkan] Error: VkResult = {0}\n",err);
	// fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace engine {
	void VulkanRHI::Init()
	{
		//**********************************************
		//modify pAllocator, Instance extensions etc here
		//**********************************************
		allocator_ = nullptr;
		uint32_t extensions_count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);

		Setup(extensions, extensions_count);
	}

	void VulkanRHI::Setup(const char** extensions, uint32_t extensions_count)
	{
		CreateInstance(extensions, extensions_count);
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

    void VulkanRHI::CreateInstance(const char** extensions, uint32_t extensions_count)
    {
		VkResult result;
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.enabledExtensionCount = extensions_count;
		create_info.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
		// Enabling validation layers
		const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
		create_info.enabledLayerCount = 1;
		create_info.ppEnabledLayerNames = layers;

		// Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
		const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
		memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
		extensions_ext[extensions_count] = "VK_EXT_debug_report";
		create_info.enabledExtensionCount = extensions_count + 1;
		create_info.ppEnabledExtensionNames = extensions_ext;

		// Create Vulkan Instance
		result = vkCreateInstance(&create_info, allocator_, &instance_);
        check_vk_result(result);
#else
		// Create Vulkan Instance without any debug feature
		err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
		check_vk_result(err);
		IM_UNUSED(g_DebugReport);
#endif
    }

	void VulkanRHI::PickPhysicalDevice()
	{
		uint32_t gpu_count;
		VkResult result;
		result = vkEnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
		check_vk_result(result);
		// IM_ASSERT(gpu_count > 0);

		std::vector<VkPhysicalDevice> gpus(gpu_count);
		result = vkEnumeratePhysicalDevices(instance_, &gpu_count, gpus.data());
		check_vk_result(result);
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		for(auto gpu : gpus)
		{
			vkGetPhysicalDeviceFeatures(gpu, &features);
			vkGetPhysicalDeviceProperties(gpu, &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && 
				features.geometryShader)
			{
				physical_device_ = gpu;
				break;
			}
		}
	}

	void VulkanRHI::CreateLogicalDevice()
	{
		VkResult result;
		//device related
		int device_extension_count = 1;
		const char* device_extensions[] = { "VK_KHR_swapchain" };
		//queue related
		uint32_t queue_family_index;
		VkDeviceQueueCreateInfo queue_create_info[1] = {};
		const float queue_priority[] = { 1.0f };
		//create device queue family
		{
			uint32_t count;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &count, nullptr);
			VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &count, queues);
			for(uint32_t i = 0; i < count; i++)
			{
				if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queue_family_index = i;
					break;
				}
			}
			free(queues);

			queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info[0].queueFamilyIndex = queue_family_index;
			queue_create_info[0].queueCount = 1;
			queue_create_info[0].pQueuePriorities = queue_priority;
		}
		//create device
		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = sizeof(queue_create_info) / sizeof(queue_create_info[0]);
		create_info.pQueueCreateInfos = queue_create_info;
		create_info.enabledExtensionCount = device_extension_count;
		create_info.ppEnabledExtensionNames = device_extensions;
		result = vkCreateDevice(physical_device_, &create_info, allocator_, &device_);
		check_vk_result(result);
		vkGetDeviceQueue(device_, queue_family_index, 0, &graphics_queue_);
	}

	void VulkanRHI::Shutdown()
	{

	}
}