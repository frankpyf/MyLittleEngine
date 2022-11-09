#pragma once
#include <vulkan/vulkan.h>
#include "Runtime/Function/RHI/CommandBuffer.h"

namespace rhi {
	class VulkanDevice;
	class VulkanCommandEncoder
	{
	public:
		virtual void Begin();

		virtual void End();
	protected:
		VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
	};

	class VulkanGraphicsEncoder : public RHIGraphicsEncoder, public VulkanCommandEncoder
	{
		friend class VulkanCommandBuffer;
	public:
		virtual ~VulkanGraphicsEncoder();
		virtual void BeginRenderPass(renderer::RenderPass& pass,
									 renderer::RenderTarget& render_target) override;
		virtual void BindGfxPipeline(renderer::Pipeline* pipeline) override;
		virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
		virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) override;
		virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;

		virtual void NextSubpass() override;

		virtual void EndRenderPass() override;

		virtual void ImGui_RenderDrawData(ImDrawData* draw_data) override;
	};

	class VulkanCommandBuffer : public CommandBuffer
	{
		
	public:
		VulkanCommandBuffer(VulkanDevice* in_device);
		virtual ~VulkanCommandBuffer();
		virtual void AllocateCommandBuffers() override;
		virtual void Begin() override;
		virtual void End() override;
		inline virtual RHIGraphicsEncoder& GetGfxEncoder()	override { return gfx_encoder_; };
		inline virtual void* GetNativeGfxHandle()			override { return (void*)gfx_encoder_.command_buffer_; };
		inline virtual void* GetImageAvailableSemaphore()	override { return (void*)image_acquired_semaphore_; };
		inline virtual void* GetRenderFinishedSemaphore()	override { return (void*)render_finished_semaphore_; };
		inline virtual void* GetInFlightFence()				override { return (void*)frame_in_flight_; };
	private:
		VulkanDevice* device_;
		VkCommandPool command_pool_ = VK_NULL_HANDLE;

		VulkanGraphicsEncoder gfx_encoder_;

		VkFence frame_in_flight_ = VK_NULL_HANDLE;

		VkSemaphore image_acquired_semaphore_ = VK_NULL_HANDLE;
		VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
	};
}

