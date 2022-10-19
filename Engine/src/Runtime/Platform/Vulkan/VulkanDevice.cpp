#include "mlepch.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"
#include "Runtime/Core/Base/Log.h"

#define ARRAY_SIZE( ARRAY ) (sizeof (ARRAY) / sizeof (ARRAY[0]))

namespace rhi {
	VulkanDevice::VulkanDevice(VulkanRHI* in_rhi, VkPhysicalDevice in_gpu)
		:gpu_(in_gpu),
		device_(VK_NULL_HANDLE),
		graphics_queue_(VK_NULL_HANDLE),
		compute_queue_(VK_NULL_HANDLE),
		transfer_queue_(VK_NULL_HANDLE),
		present_queue_(VK_NULL_HANDLE)
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
		vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
		vkDestroyCommandPool(device_, command_pool_, nullptr);

		delete graphics_queue_;
		delete compute_queue_;
		delete transfer_queue_;

		if (device_ != VK_NULL_HANDLE)
			vkDestroyDevice(device_, nullptr);

		MLE_CORE_INFO("device has been destroyed");
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
					MLE_CORE_INFO("Initializing Gfx Queue with Queue family {0} which has {1} queues", family_index, curr_props.queueCount);
				}
			}

			if (curr_props.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				if (compute_queue_family_index == -1 && gfx_queue_family_index != family_index)
				{
					compute_queue_family_index = family_index;
					is_valid_queue = true;
					MLE_CORE_INFO("Initializing Compute Queue with Queue family {0} which has {1} queues", family_index, curr_props.queueCount);
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
					MLE_CORE_INFO("Initializing Transfer Queue with Queue family {0} which has {1} queues", family_index, curr_props.queueCount);
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

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void VulkanDevice::CreateDescriptorPool()
	{
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * (int)(sizeof(pool_sizes)/sizeof(*pool_sizes));
		pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(*pool_sizes));
		pool_info.pPoolSizes = pool_sizes;
		VkResult result = vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
		MLE_CORE_INFO("Global descriptor pool has been created!");
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
		CreateDescriptorPool();
		CreateCommandPool();
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
					MLE_CORE_INFO("Queue Family {0}: Supports Present", family_index);
				}
				return (supports_present == VK_TRUE);
			};

			bool gfx = SupportsPresent(gpu_, graphics_queue_);

			bool compute = SupportsPresent(gpu_, compute_queue_);

			if (compute_queue_->GetFamilyIndex() != graphics_queue_->GetFamilyIndex() && compute)
				present_queue_ = compute_queue_;
			else
				present_queue_ = graphics_queue_;
			MLE_CORE_INFO("Present Queue family index: {0}", present_queue_->GetFamilyIndex());
		}
	}

	VkCommandBuffer VulkanDevice::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool_;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device_, &alloc_info, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanDevice::EndSingleTimeCommands(VkCommandBuffer command_buffer)
	{
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &command_buffer;

		vkQueueSubmit(graphics_queue_->GetQueueHandle(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue_->GetQueueHandle());

		vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer);
	}
}