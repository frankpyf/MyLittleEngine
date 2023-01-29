#include "mlepch.h"
#include "VulkanRHI.h"
#include "VulkanUtils.h"
#include "VulkanRenderPass.h"
#include "VulkanResource.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptor.h"

#include <vector>
#include <GLFW/glfw3.h>
#include "Runtime/Core/Base/Log.h"
#include "Runtime/Core/Base/Application.h"
#include "Runtime/Core/Window.h"

#include "Runtime/Resource/Vertex.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

#ifdef MLE_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	MLE_CORE_ERROR("[vulkan] Debug report from ObjectType: {0}\nMessage: {1}\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // MLE_DEBUG
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;

namespace utils {
	FORCEINLINE static VkBufferUsageFlags ResolveBufferUsage(ResourceTypes type)
	{
		VkBufferUsageFlags usage = 0;
		if(EnumHasFlag(type, ResourceTypes::RESOURCE_TYPE_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (EnumHasFlag(type, ResourceTypes::RESOURCE_TYPE_UNIFORM_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (EnumHasFlag(type, ResourceTypes::RESOURCE_TYPE_INDEX_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (EnumHasFlag(type, ResourceTypes::RESOURCE_TYPE_VERTEX_BUFFER))
		{
			usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		return usage;
	}


	std::vector<char> ReadFile(const char* file_path)
	{
		std::ifstream file(file_path, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
}


namespace rhi {

	void VulkanRHI::Init()
	{
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
		CreateVulkanMemoryAllocator();

		engine::Application& app = engine::Application::GetApp();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		viewport_ = new VulkanViewport(this, device_, &app.GetWindow());

		depth_format_ = VulkanUtils::FindDepthFormat(device_->GetPhysicalHandle());
	}


	void VulkanRHI::Shutdown()
	{
		RHIBlockUntilGPUIdle();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vmaDestroyAllocator(allocator_);

		viewport_->Destroy();
#ifdef MLE_DEBUG
		// Remove the debug report callback
		auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(instance_, g_DebugReport, nullptr);
#endif // MLE_DEBUG
		
		device_->Destroy();

		vkDestroyInstance(instance_, nullptr);

		MLE_CORE_INFO("Vulkan RHI has been shut down");
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
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "My Little Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &appInfo;
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
#ifdef MLE_DEBUG
		// Get the function pointer (required for any extensions)
		auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");
		IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

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
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to enumerate physical devices");
		}
		assert(gpu_count > 0 && "No GPU that support Vulkan is found!");

		VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
		result = vkEnumeratePhysicalDevices(instance_, &gpu_count, gpus);
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to enumerate physical devices");
		}

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

		assert(device_ != nullptr && "No proper device found!");

		device_->InitGPU();
	}

	void VulkanRHI::CreateVulkanMemoryAllocator()
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorCreateInfo.physicalDevice = device_->GetPhysicalHandle();
		allocatorCreateInfo.device = device_->GetDeviceHandle();
		allocatorCreateInfo.instance = instance_;
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		if (vmaCreateAllocator(&allocatorCreateInfo, &allocator_) != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to create VMA allocator");
			throw std::runtime_error("Failed to create VMA allocator");
		}
		MLE_CORE_INFO("VMA allocator created");
	}

	void VulkanRHI::RHITick(float delta_time)
	{

	}

	void VulkanRHI::RHIBlockUntilGPUIdle()
	{
		vkDeviceWaitIdle(device_->GetDeviceHandle());
	}

	void* VulkanRHI::GetNativeInstance()
	{
		return (void*)GetVkInstance();
	}

	void* VulkanRHI::GetNativeDevice()
	{
		return (void*)device_->GetDeviceHandle();
	}

	void* VulkanRHI::GetNativePhysicalDevice()
	{
		return (void*)device_->GetPhysicalHandle();
	}

	void* VulkanRHI::GetNativeGraphicsQueue()
	{
		return (void*)device_->GetGfxQueue()->GetQueueHandle();
	}

	void* VulkanRHI::GetNativeComputeQueue()
	{
		return (void*)device_->GetComputeQueue()->GetQueueHandle();
	}

	void VulkanRHI::AcquireNextImage(Semaphore* semaphore)
	{
		VulkanSemaphore* image_acquired_semaphore = (VulkanSemaphore*)semaphore;
		viewport_->AcquireNextImage(image_acquired_semaphore->semaphore);
	}

	void* VulkanRHI::GetNativeSwapchainImageView()
	{
		uint32_t index = viewport_->GetAccquiredIndex();
		return (void*)viewport_->GetSwapChain()->GetSwapchianImageView(index);
	}

	uint32_t VulkanRHI::GetViewportWidth()
	{
		return viewport_->GetViewportWidth();
	}

	uint32_t VulkanRHI::GetViewportHeight()
	{
		return viewport_->GetViewportHeight();
	}

	uint32_t VulkanRHI::GetGfxQueueFamily()
	{
		return device_->GetGfxQueue()->GetFamilyIndex();
	}

	void VulkanRHI::GfxQueueSubmit(const QueueSubmitDesc& desc)
	{
		device_->GetGfxQueue()->Submit(desc);
	}

	void VulkanRHI::ComputeQueueSubmit(const QueueSubmitDesc& desc)
	{

	}

	void VulkanRHI::TransferQueueSubmit(const QueueSubmitDesc& desc)
	{
		device_->GetTransferQueue()->Submit(desc);
	}

	void VulkanRHI::Present(Semaphore** semaphores, uint32_t semaphore_count)
	{
		viewport_->Present(semaphores, semaphore_count);
	}

	DescriptorSet* VulkanRHI::CreateDescriptorSet()
	{
		return new VulkanDescriptorSet;
	}

	DescriptorSetLayout* VulkanRHI::CreateDescriptorSetLayout()
	{
		return new VulkanDescriptorSetLayout;
	}

	DescriptorSetLayoutCache* VulkanRHI::CreateDescriptorSetLayoutCache()
	{
		return new VulkanDescriptorSetLayoutCache(device_);
	}
	DescriptorAllocator* VulkanRHI::CreateDescriptorAllocator()
	{
		return new VulkanDescriptorAllocator(device_);
	}

	CommandBuffer* VulkanRHI::RHICreateCommandBuffer()
	{
		return new VulkanCommandBuffer(device_);
	}

	std::shared_ptr<RHITexture2D> VulkanRHI::RHICreateTexture2D(const RHITexture2D::Descriptor& desc)
	{
		return std::make_shared<VulkanTexture2D>(*this, desc);
	}

	std::shared_ptr<RHITexture2D> VulkanRHI::RHICreateTexture2D(std::string_view path, uint32_t miplevels)
	{
		return std::make_shared<VulkanTexture2D>(*this, path, miplevels);
	}

	RenderPass* VulkanRHI::RHICreateRenderPass(const RenderPass::Descriptor& desc)
	{
		return new VulkanRenderPass(*this, desc);
	}

	std::unique_ptr<RenderTarget> VulkanRHI::RHICreateRenderTarget(const RenderTarget::Descriptor& desc)
	{
		return std::make_unique<VulkanRenderTarget>(*this, desc);
	}

	Semaphore* VulkanRHI::RHICreateSemaphore()
	{
		VulkanSemaphore* semaphore_vk = new VulkanSemaphore{};
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(device_->GetDeviceHandle(), &semaphoreInfo, nullptr, &semaphore_vk->semaphore) != VK_SUCCESS) 
		{
			MLE_CORE_ERROR("Failed to create vulkan semaphore");
			throw std::runtime_error("Failed to create vulkan semaphore");
		}
		return semaphore_vk;
	}

	Fence* VulkanRHI::RHICreateFence()
	{
		VulkanFence* fence_vk = new VulkanFence{};
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (vkCreateFence(device_->GetDeviceHandle(), &fenceInfo, nullptr, &fence_vk->fence) != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to create vulkan fence");
			throw std::runtime_error("Failed to create vulkan fence");
		}
		return fence_vk;
	}

	void VulkanRHI::RHIDestroySemaphore(Semaphore* semaphore)
	{
		VulkanSemaphore* semaphore_vk = (VulkanSemaphore*)semaphore;
		vkDestroySemaphore(device_->GetDeviceHandle(), semaphore_vk->semaphore, nullptr);
		delete semaphore;
	}

	void VulkanRHI::RHIDestroyFence(Fence* fence)
	{
		VulkanFence* fence_vk = (VulkanFence*)fence;
		vkDestroyFence(device_->GetDeviceHandle(), fence_vk->fence, nullptr);
		delete fence;
	}

	void VulkanRHI::RHIWaitForFences(Fence** fence, uint32_t fence_count)
	{
		VkFence* fences = new VkFence[fence_count];
		for (uint32_t i = 0; i < fence_count; ++i)
		{
			VulkanFence* fence_vk = (VulkanFence*)fence[i];
			fences[i] = fence_vk->fence;
		}
		vkWaitForFences(device_->GetDeviceHandle(), fence_count, fences, VK_TRUE, UINT64_MAX);
		vkResetFences(device_->GetDeviceHandle(), fence_count, fences);
		delete[] fences;
	}

	bool VulkanRHI::RHIIsFenceReady(Fence* fence)
	{
		VulkanFence* fence_rhi = static_cast<VulkanFence*>(fence);
		VkFence vk_fence = fence_rhi->fence;
		VkResult result = vkGetFenceStatus(device_->GetDeviceHandle(), vk_fence);
		switch (result)
		{
		case VK_SUCCESS: return true;
		case VK_NOT_READY: return false;
		case VK_ERROR_DEVICE_LOST: MLE_CORE_ERROR("[vulkan] The device has been lost"); throw std::runtime_error("[vulkan] The device has been lost");
		}
		return false;
	}

	// ---------------------------------Resource Creation and deconstruction-----------------------------------
	BufferRef VulkanRHI::RHICreateBuffer(const RHIBuffer::Descriptor& desc)
	{
		// TODO: Add Support for Dynamic Buffer
		auto buffer = std::make_shared<VulkanBuffer>();
		buffer->size = desc.size;
		buffer->usage = desc.usage;

		// -------------Resolve Buffer usage-------------
		VkBufferUsageFlags usage = utils::ResolveBufferUsage(desc.usage);

		// if this buffer needs staging buffer to upload data or it needs to read data from the gpu 
		if (desc.memory_usage == MemoryUsage::MEMORY_USAGE_GPU_ONLY)
			usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// -------------Resolve VMA Create Info, annnnnnd fix some usage stuff-------------
		VmaAllocationCreateInfo vma_create_info{};
		// This is the way
		vma_create_info.usage =
			desc.prefer_device ? VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE :
			desc.prefer_host ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO;

		if(desc.memory_usage == MemoryUsage::MEMORY_USAGE_CPU_TO_GPU)
		{
			vma_create_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (desc.memory_usage == MemoryUsage::MEMORY_USAGE_GPU_TO_CPU)
		{
			vma_create_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		if (desc.mapped_at_creation)
			vma_create_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VulkanUtils::VMACreateBuffer(allocator_,
			desc.size,
			usage,
			buffer->buffer,
			buffer->buffer_allocation,
			desc.mapped_at_creation ? &buffer->alloc_info : nullptr,
			vma_create_info
			);

		buffer->buffer_info.buffer = buffer->buffer;
		buffer->buffer_info.offset = 0;
		buffer->buffer_info.range = buffer->size;

#ifdef MLE_DEBUG
		MLE_CORE_INFO("[vulkan] Vertex Buffer created");
#endif // MLE_DEBUG
		return buffer;
	}

	void VulkanRHI::RHIFreeBuffer(BufferRef buffer)
	{
		VulkanBuffer* vk_buffer = static_cast<VulkanBuffer*>(buffer.get());
		if (vk_buffer->buffer != VK_NULL_HANDLE)
			vmaDestroyBuffer(allocator_, vk_buffer->buffer, vk_buffer->buffer_allocation);
	}

	ShaderModule* VulkanRHI::RHICreateShaderModule(const char* path)
	{
		auto code = utils::ReadFile(path);
		VulkanShaderModule* vk_shader = new VulkanShaderModule();

		VkShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = code.size();
		shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkResult result;
		result = vkCreateShaderModule(device_->GetDeviceHandle(), &shader_module_create_info, nullptr, &vk_shader->shader_module);
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to create ShaderModule");
		}

		return vk_shader;
	}

	void VulkanRHI::RHIFreeShaderModule(ShaderModule* shader)
	{
		VulkanShaderModule* vk_shader = (VulkanShaderModule*)shader;
		vkDestroyShaderModule(device_->GetDeviceHandle(), vk_shader->shader_module, nullptr);
	}

	PipelineLayout* VulkanRHI::RHICreatePipelineLayout(const PipelineLayout::Descriptor& desc)
	{
		VulkanPipelineLayout* pipeline_layout = new VulkanPipelineLayout{};
		VulkanDescriptorSetLayout** vk_layouts = (VulkanDescriptorSetLayout**)desc.layouts;
		VkDescriptorSetLayout actual_layouts[64];
		for (uint32_t i = 0; i < desc.set_layout_count; ++i)
		{
			actual_layouts[i] = vk_layouts[i]->layout;
		}
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = desc.set_layout_count;
		pipelineLayoutInfo.pSetLayouts = actual_layouts;
		pipelineLayoutInfo.pushConstantRangeCount = desc.push_constant_count;

		if (vkCreatePipelineLayout(device_->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &pipeline_layout->pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		return pipeline_layout;
	}

	void VulkanRHI::RHIFreePipelineLaoyout(PipelineLayout* layout)
	{
		VulkanPipelineLayout* pipeline_layout = (VulkanPipelineLayout*)layout;
		vkDestroyPipelineLayout(device_->GetDeviceHandle(), pipeline_layout->pipeline_layout, nullptr);
	}

	PipelineRef VulkanRHI::RHICreatePipeline(const RHIPipeline::Descriptor& desc)
	{
		assert(desc.layout != nullptr && "Cannot create graphics pipeline: no pipeline layout provided in desc");
		auto new_pipeline = std::make_shared<VulkanPipeline>();

		VulkanPipelineLayout* vk_layout = (VulkanPipelineLayout*)desc.layout;
		VulkanShaderModule* vk_vert = (VulkanShaderModule*)desc.vert_shader;
		VulkanShaderModule* vk_frag = (VulkanShaderModule*)desc.frag_shader;
		
		new_pipeline->layout = vk_layout;
		new_pipeline->vert_shader = vk_vert->shader_module;
		new_pipeline->frag_shader = vk_frag->shader_module;


		assert(
			desc.render_pass != nullptr &&
			"Cannot create graphics pipeline: no renderPass provided in desc");
		
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = new_pipeline->vert_shader;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = new_pipeline->frag_shader;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// TEMP
		VkVertexInputBindingDescription binding_description{};
		binding_description.binding = 0;
		binding_description.stride = sizeof(resource::Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription position{};
		position.binding = 0;
		position.location = 0;
		position.format = VK_FORMAT_R32G32B32_SFLOAT;
		position.offset = offsetof(resource::Vertex, pos);

		VkVertexInputAttributeDescription color{};
		color.binding = 0;
		color.location = 1;
		color.format = VK_FORMAT_R32G32B32_SFLOAT;
		color.offset = offsetof(resource::Vertex, color);

		VkVertexInputAttributeDescription vertex_attribute_desc[] = { position, color };
		

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = desc.use_vertex_attribute ? 2 : 0;
		vertexInputInfo.vertexBindingDescriptionCount = desc.use_vertex_attribute ? 1 : 0;
		vertexInputInfo.pVertexAttributeDescriptions = desc.use_vertex_attribute ? vertex_attribute_desc : nullptr;
		vertexInputInfo.pVertexBindingDescriptions = desc.use_vertex_attribute ? &binding_description : nullptr;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
		input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = nullptr;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterization_state{};
		rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state.depthClampEnable = VK_FALSE;
		rasterization_state.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_state.lineWidth = 1.0f;
		rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_state.depthBiasEnable = VK_FALSE;
		rasterization_state.depthBiasConstantFactor = 0.0f;  // Optional
		rasterization_state.depthBiasClamp = 0.0f;           // Optional
		rasterization_state.depthBiasSlopeFactor = 0.0f;     // Optional


		VkPipelineMultisampleStateCreateInfo multisample_state{};
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.sampleShadingEnable = VK_FALSE;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.minSampleShading = 1.0f;           // Optional
		multisample_state.pSampleMask = nullptr;             // Optional
		multisample_state.alphaToCoverageEnable = VK_FALSE;  // Optional
		multisample_state.alphaToOneEnable = VK_FALSE;       // Optional

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		VkPipelineColorBlendStateCreateInfo color_blend_state{};
		color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state.logicOpEnable = VK_FALSE;
		color_blend_state.logicOp = VK_LOGIC_OP_COPY;  // Optional
		color_blend_state.attachmentCount = 1;
		color_blend_state.pAttachments = &color_blend_attachment;
		color_blend_state.blendConstants[0] = 0.0f;  // Optional
		color_blend_state.blendConstants[1] = 0.0f;  // Optional
		color_blend_state.blendConstants[2] = 0.0f;  // Optional
		color_blend_state.blendConstants[3] = 0.0f;  // Optional

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.depthTestEnable = VK_TRUE;
		depth_stencil_state.depthWriteEnable = VK_TRUE;
		depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.minDepthBounds = 0.0f;  // Optional
		depth_stencil_state.maxDepthBounds = 1.0f;  // Optional
		depth_stencil_state.stencilTestEnable = VK_FALSE;
		depth_stencil_state.front = {};  // Optional
		depth_stencil_state.back = {};   // Optional

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.pDynamicStates = dynamicStateEnables.data();
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamic_state.flags = 0;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &input_assembly_state;
		pipelineInfo.pViewportState = &viewport_state;
		pipelineInfo.pRasterizationState = &rasterization_state;
		pipelineInfo.pMultisampleState = &multisample_state;
		pipelineInfo.pColorBlendState = &color_blend_state;
		pipelineInfo.pDepthStencilState = &depth_stencil_state;
		pipelineInfo.pDynamicState = &dynamic_state;

		pipelineInfo.layout = vk_layout->pipeline_layout;
		pipelineInfo.renderPass = (VkRenderPass)desc.render_pass->GetHandle();
		pipelineInfo.subpass = desc.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(
			device_->GetDeviceHandle(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&new_pipeline->pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		}

		return new_pipeline;
	}

	void VulkanRHI::RHIFreePipeline(RHIPipeline* pipeline)
	{
		VulkanPipeline* vk_pipeline = (VulkanPipeline*)pipeline;

		vkDestroyPipeline(device_->GetDeviceHandle(), vk_pipeline->pipeline, nullptr);
	}
}