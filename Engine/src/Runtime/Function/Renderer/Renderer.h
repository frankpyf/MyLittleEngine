#pragma once
#include "Runtime/Platform/Vulkan/VulkanRHI.h"
namespace engine {
	/**
	* 
	*/
	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		int GetFrameIndex() const
		{
			assert(is_frame_started_);
			return current_frame_index_;
		}
		VulkanRHI* GetRHI() { return rhi_.get(); }

		VkCommandBuffer GetCurrentCommandBuffer() const 
		{
			assert(is_frame_started_ && "Cannot get command buffer when frame not in progress");
			return command_buffers_[current_frame_index_];
		}

		void Init();
		void Shutdown();
		void Tick();

		VkCommandBuffer BeginFrame();
		void EndFrame();

		void Resize(uint32_t width, uint32_t height);
	private:
		virtual void CreateCommandBuffers();
		virtual void FreeCommandBuffers();

		std::unique_ptr<VulkanRHI> rhi_;
		std::vector<VkCommandBuffer> command_buffers_;
		uint32_t current_image_index_;
		int current_frame_index_{0};
		bool is_frame_started_{ false };
	};
}

