#pragma once
#include <vulkan/vulkan.h>
#include "Runtime/Function/RHI/CommandBuffer.h"
#include "VulkanResource.h"

namespace rhi {
	class VulkanDevice;
	class VulkanEncoderBase
	{
		friend class VulkanQueue;
	public:
		virtual ~VulkanEncoderBase() = default;
		virtual void InternalBegin();
		virtual void InternalEnd();
	protected:
		VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
	};

	class VulkanGraphicsEncoder : public RHIGraphicsEncoder, public VulkanEncoderBase
	{
		friend class VulkanCommandBuffer;
		friend class VulkanQueue;
	public:
		virtual ~VulkanGraphicsEncoder() = default;

		virtual void Begin() override { InternalBegin(); }
		virtual void BeginRenderPass(renderer::RenderPass& pass,
									 renderer::RenderTarget& render_target) override;
		virtual void BindGfxPipeline(renderer::Pipeline* pipeline) override;
		virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
		virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) override;
		virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;

		virtual void NextSubpass() override;

		virtual void EndRenderPass() override;

		virtual void ImGui_RenderDrawData(ImDrawData* draw_data) override;

		virtual void End() override { InternalEnd(); };

		virtual void* GetHandle() override { return (void*)command_buffer_; };
	};

	class VulkanTransferEncoder :public RHITransferEncoder, public VulkanEncoderBase
	{
		friend class VulkanCommandBuffer;
		friend class VulkanQueue;
	public:
		virtual ~VulkanTransferEncoder() = default;
		virtual void Begin() override { InternalBegin(); };
		virtual void CopyBufferToBuffer(const CopyBufferToBufferDesc& desc)	override;
		virtual void CopyBufferToImage(RHIBuffer* buffer,
									   RHITexture2D* image,
									   uint32_t              width,
									   uint32_t              height,
									   uint32_t              layer_count)	override;
		virtual void End() override { InternalEnd(); };

		virtual void* GetHandle() override { return (void*)command_buffer_; };
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
		virtual RHITransferEncoder& GetTransferEncoder()	override { return transfer_encoder_; };
		virtual void* GetNativeTransferHandle()				override { return (void*)transfer_encoder_.command_buffer_; };
	private:
		VulkanDevice* device_;
		VkCommandPool command_pool_ = VK_NULL_HANDLE;

		VulkanGraphicsEncoder gfx_encoder_;
		VulkanTransferEncoder transfer_encoder_;

	};
}

