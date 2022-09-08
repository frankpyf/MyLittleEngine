#include "mlepch.h"
#include "VulkanRHI.h"
#include <vector>
#include <GLFW/glfw3.h>
#include "Runtime/Core/Base/Log.h"
#include "Runtime/Core/Base/Application.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "VulkanDevice.h"

#ifdef MLE_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // MLE_DEBUG
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;


namespace engine {
	VulkanRHI::VulkanRHI()
		:instance_(VK_NULL_HANDLE),
		device_(nullptr),
		viewport_(nullptr)
	{
	}

	void VulkanRHI::Init()
	{
		//**********************************************
		//modify Instance extensions etc here
		//**********************************************
		/*uint32_t extensions_count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);*/

		// Setup Vulkan
		if (!glfwVulkanSupported())
		{
			MLE_CORE_ERROR("GLFW: Vulkan not supported!\n");
			return;
		}

		if (!device_)
		{
			CreateInstance();
			SelectAndInitDevice();
		}
	}


	void VulkanRHI::Shutdown()
	{
#ifdef MLE_DEBUG
		// Remove the debug report callback
		auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(instance_, g_DebugReport, nullptr);
#endif // MLE_DEBUG

		
		device_->Destroy();
		delete device_;

		vkDestroyInstance(instance_, nullptr);
	}

	void VulkanRHI::GetExtensionsAndLayers()
	{
		uint32_t extensions_count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
		for (uint32_t i = 0; i < extensions_count; ++i)
		{
			instance_extensions_.emplace_back(extensions[i]);
		}
#ifdef MLE_DEBUG
		instance_extensions_.emplace_back("VK_EXT_debug_report");
#endif // MLE_DEBUG

		instance_layers_.emplace_back("VK_LAYER_KHRONOS_validation");
	}

    void VulkanRHI::CreateInstance()
    {
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		GetExtensionsAndLayers();

		create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions_.size());
		create_info.ppEnabledExtensionNames = create_info.enabledExtensionCount > 0 ?
											  instance_extensions_.data() : nullptr;

		// Enabling validation layers
		create_info.enabledLayerCount = static_cast<uint32_t>(instance_layers_.size());
		create_info.ppEnabledLayerNames = create_info.enabledLayerCount > 0 ?
										  instance_layers_.data() : nullptr;

		// Create Vulkan Instance
		VkResult result = vkCreateInstance(&create_info, nullptr, &instance_);
		if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
		{
			MLE_CORE_ERROR("Cannot find a compatible Vulkan driver (ICD)");
			abort();
		}
		else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
		{
			// Check for missing extensions
			std::string missing_extensions;

			uint32_t property_count;
			vkEnumerateInstanceExtensionProperties(nullptr, &property_count, nullptr);

			std::vector<VkExtensionProperties> properties(property_count);
			vkEnumerateInstanceExtensionProperties(nullptr, &property_count, properties.data());

			for (const char* extension : instance_extensions_)
			{
				bool extension_found = false;
				for (uint32_t property_index = 0; property_index < property_count; ++property_count)
				{
					const char* property_extension_name = properties[property_index].extensionName;
					if (!strcmp(property_extension_name, extension))
					{
						extension_found = true;
						break;
					}
				}
				
				if (!extension_found)
				{
					std::string extension_str(extension);
					MLE_CORE_ERROR("Missing required Vulkan extension: {0}", extension_str);
					missing_extensions += extension + '\n';
				}

				MLE_CORE_ERROR("Vulkan driver doesn't contain specified extensions: {0}", missing_extensions);
				abort();
			}
		}
		else if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("failed to create instance");
			abort();
		}

		// Get the function pointer (required for any extensions)
		auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");
		IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

#ifdef MLE_DEBUG
		// Setup the debug report callback
		VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
		debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debug_report_ci.pfnCallback = debug_report;
		debug_report_ci.pUserData = NULL;
		result = vkCreateDebugReportCallbackEXT(instance_, &debug_report_ci, nullptr, &g_DebugReport);
		if (result != VK_SUCCESS) 
		{
			MLE_CORE_ERROR("Failed to create debug report");
			abort();
		}
#endif // MLE_DEBUG
    }

	void VulkanRHI::SelectAndInitDevice()
	{
		uint32_t gpu_count = 0;
		VkResult result = vkEnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
		check_vk_result(result);
		assert(gpu_count > 0, "No GPU that support Vulkan is found!");

		VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		result = vkEnumeratePhysicalDevices(instance_, &gpu_count, gpus);
		check_vk_result(result);

		//struct DeviceInfo
		//{
		//	VulkanDevice* device;
		//	uint32_t device_index;
		//};
		//std::vector<DeviceInfo> discrete_devices;
		//std::vector<DeviceInfo> integrated_devices;

		MLE_CORE_INFO("Found {0} device(s)", gpu_count);
		//enumerate all physical device
		for(int i = 0; i < (int)gpu_count; i++)
		{
			VulkanDevice* new_device = new VulkanDevice(this, gpus[i]);

			bool is_discrete = new_device->QueryGPU();

			if (is_discrete)
			{
				//discrete_devices.emplace_back(new_device, i);
				device_ = new_device;
				break;
			}
			/*else
			{

				integrated_devices.emplace_back(new_device, i);
			}*/
			delete new_device;
		}
		free(gpus);

		//Pick the first discrete device
		//device_ = discrete_devices[0].device;

		assert(device_ != nullptr, "No proper device found!");

		device_->InitGPU();
	}

	void VulkanRHI::RHITick(float delta_time)
	{

	}

	void VulkanRHI::RHIBlockUntilGPUIdle()
	{
		vkDeviceWaitIdle(device_->GetDeviceHandle());
	}
}