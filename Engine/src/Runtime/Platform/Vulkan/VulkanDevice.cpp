#include "mlepch.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "Runtime/Core/Base/Log.h"

#define ARRAY_SIZE( ARRAY ) (sizeof (ARRAY) / sizeof (ARRAY[0]))

namespace engine {
	VulkanDevice::VulkanDevice(VulkanRHI* in_rhi, VkPhysicalDevice in_gpu)
		:gpu_(in_gpu),
		device_(VK_NULL_HANDLE),
		graphics_queue_(VK_NULL_HANDLE),
		compute_queue_(VK_NULL_HANDLE),
		transfer_queue_(VK_NULL_HANDLE),
		present_queue_(VK_NULL_HANDLE),
		command_pool_(VK_NULL_HANDLE)
	{
		rhi_ = in_rhi;
	}
	
	VulkanDevice::~VulkanDevice()
	{
		if (device_ != VK_NULL_HANDLE)
			Destroy();
		device_ = VK_NULL_HANDLE;
	}

	void VulkanDevice::Destroy()
	{
		// todo:delete 3 VulkanQueue pointers
		delete graphics_queue_;
		delete compute_queue_;

		if (device_ != VK_NULL_HANDLE)
			vkDestroyDevice(device_, nullptr);
	}
	void VulkanDevice::CreateLogicalDevice()
	{
		VkResult result;
		assert(device_ == VK_NULL_HANDLE);

		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// extensions related
		int device_extension_count = 1;
		const char* device_extensions[] = { "VK_KHR_swapchain" };
		device_create_info.enabledExtensionCount = device_extension_count;
		device_create_info.ppEnabledExtensionNames = device_extensions;

		// validation layer
		device_create_info.enabledLayerCount = 0;
		device_create_info.ppEnabledLayerNames = nullptr;

		// Setup queue info
		std::vector<VkDeviceQueueCreateInfo> queue_create_info;
		const float queue_priority[] = { 1.0f };
		int gfx_queue_family_index = -1;
		int compute_queue_family_index = -1;
		int transfer_queue_family_index = -1;
		MLE_CORE_INFO("Found {0} Queue Families",queue_family_props_.size());
		//select device queue family
		for (uint32_t family_index = 0; family_index < queue_family_props_.size(); ++family_index)
		{
			const VkQueueFamilyProperties& curr_props = queue_family_props_[family_index];
			bool is_valid_queue = false;
			if (curr_props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (gfx_queue_family_index == -1)
				{
					gfx_queue_family_index = family_index;
					is_valid_queue = true;
				}
			}

			if (curr_props.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				if (compute_queue_family_index == -1 && gfx_queue_family_index != family_index)
				{
					compute_queue_family_index = family_index;
					is_valid_queue = true;
				}
			}

			if (curr_props.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				if (transfer_queue_family_index == -1 &&
					(curr_props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT &&
					(curr_props.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
				{
					transfer_queue_family_index = family_index;
					is_valid_queue = true;
				}
			}
			if (!is_valid_queue)
			{
				MLE_CORE_WARN("Skipping unnecessary Queue Family {0}: {1} queues", family_index, curr_props.queueCount);
				continue;
			}
			int queue_index = queue_create_info.size();
			queue_create_info.resize(queue_index + 1);
			VkDeviceQueueCreateInfo& curr_queue = queue_create_info[queue_index];
			curr_queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			curr_queue.queueFamilyIndex = family_index;
			//curr_queue.queueCount = curr_props.queueCount;
			curr_queue.queueCount = 1;
			curr_queue.pQueuePriorities = queue_priority;
			MLE_CORE_INFO("Initialized Queue family {0} which has {1} queues", family_index, curr_props.queueCount);
		}

		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_info.size());
		device_create_info.pQueueCreateInfos = queue_create_info.data();
		
		//Create the device
		result = vkCreateDevice(gpu_, &device_create_info, nullptr, &device_);
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("failed to create logical device");
			abort();
		}

		graphics_queue_ = new VulkanQueue(this, gfx_queue_family_index);
		if (compute_queue_family_index != -1)
			compute_queue_ = new VulkanQueue(this, compute_queue_family_index);
		else
			compute_queue_ = new VulkanQueue(this, gfx_queue_family_index);
		MLE_CORE_INFO("Logical Device is created from: {0}", gpu_properties_.deviceName);
	}

	void VulkanDevice::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = graphics_queue_->GetFamilyIndex();
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	bool VulkanDevice::QueryGPU()
	{
		vkGetPhysicalDeviceProperties(gpu_, &gpu_properties_);
		bool is_discrete = false;
		if (gpu_properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			is_discrete = true;
			MLE_CORE_INFO("device is: {0}",gpu_properties_.deviceName);
		}
		uint32_t queue_count;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu_, &queue_count, nullptr);
		queue_family_props_.resize(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu_, &queue_count, queue_family_props_.data());
		return is_discrete;
	}

	void VulkanDevice::InitGPU()
	{
		// Query features
		vkGetPhysicalDeviceFeatures(gpu_, &features_);
		if (!features_.geometryShader)
			MLE_CORE_WARN("geometry shader feature is required");
		
		CreateLogicalDevice();
		CreateCommandPool();
	}

	uint32_t VulkanDevice::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties mem_properties;
		vkGetPhysicalDeviceMemoryProperties(gpu_, &mem_properties);
		for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
			if ((type_filter & (1 << i)) &&
				(mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void VulkanDevice::CreateImageWithInfo(
		const VkImageCreateInfo& image_info,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& image_memory) 
	{
		if (vkCreateImage(device_, &image_info, nullptr, &image) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(device_, image, &mem_requirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = mem_requirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &image_memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(device_, image, image_memory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind image memory!");
		}
	}

	VkFormat VulkanDevice::FindSupportedFormat(
		const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(gpu_, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
			{
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	void VulkanDevice::SetupPresentQueue(VkSurfaceKHR in_surface)
	{
		if (!present_queue_)
		{
			const auto SupportsPresent = [in_surface](VkPhysicalDevice physical_device, VulkanQueue* Queue)
			{
				VkBool32 supports_present = VK_FALSE;
				const uint32_t family_index = Queue->GetFamilyIndex();
				vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, in_surface, &supports_present);
				if (supports_present)
				{
					MLE_CORE_INFO("Queue Family %{0}: Supports Present", family_index);
				}
				return (supports_present == VK_TRUE);
			};

			bool gfx = SupportsPresent(gpu_, graphics_queue_);

			bool compute = SupportsPresent(gpu_, compute_queue_);

			if (compute_queue_->GetFamilyIndex() != graphics_queue_->GetFamilyIndex() && compute)
				present_queue_ = compute_queue_;
			else
				present_queue_ = graphics_queue_;
		}
	}
}