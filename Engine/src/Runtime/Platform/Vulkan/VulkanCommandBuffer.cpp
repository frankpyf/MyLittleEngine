#include "mlepch.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "Runtime/Function/Renderer/RenderPass.h"
#include "Runtime/Function/Renderer/Pipeline.h"
#include "VulkanResource.h"

#include <imgui.h>
#include "backends/imgui_impl_vulkan.h"
namespace rhi {
	void VulkanEncoderBase::InternalBegin()
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(command_buffer_, &begin_info) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
	}

	void VulkanEncoderBase::InternalEnd()
	{
		vkEndCommandBuffer(command_buffer_);
	}

	void VulkanEncoderBase::AllocateCommandBuffer(VulkanDevice* device, uint32_t family_index)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = family_index;
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device->GetDeviceHandle(), &poolInfo, nullptr, &command_pool_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool_;
		alloc_info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device->GetDeviceHandle(), &alloc_info, &command_buffer_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		};
	}
	//------------------------------------Gfx Encoder------------------------------------
	void VulkanGraphicsEncoder::BeginRenderPass(renderer::RenderPass& pass,
										        renderer::RenderTarget& render_target)
	{
		VkClearValue clear_color{};
		clear_color.color.float32[0] = render_target.GetClearColor().r * render_target.GetClearColor().a;
		clear_color.color.float32[1] = render_target.GetClearColor().g * render_target.GetClearColor().a;
		clear_color.color.float32[2] = render_target.GetClearColor().b * render_target.GetClearColor().a;
		clear_color.color.float32[3] = render_target.GetClearColor().a;

		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = (VkRenderPass)pass.GetHandle();
		info.framebuffer = (VkFramebuffer)render_target.GetHandle();
		info.renderArea.extent.width = render_target.GetWidth();
		info.renderArea.extent.height = render_target.GetHeight();
		// temp
		info.clearValueCount = 2;
		info.pClearValues = &clear_color;
		vkCmdBeginRenderPass(command_buffer_, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanGraphicsEncoder::BindGfxPipeline(renderer::Pipeline* pipeline)
	{
		vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)pipeline->GetHandle());
	}

	void VulkanGraphicsEncoder::BindVertexBuffers(uint32_t first_binding, uint32_t binding_count, rhi::RHIVertexBuffer** buffer, uint64_t* offsets)
	{
		VulkanVertexBuffer** vb = (VulkanVertexBuffer**)buffer;
		VkBuffer vbs_vk[64];
		VkDeviceSize offsets_vk[64];
		for (uint32_t i = 0; i < binding_count; ++i)
		{
			vbs_vk[i] = vb[i]->buffer_;
			offsets_vk[i] = offsets ? offsets[i] : 0;
		}
		vkCmdBindVertexBuffers(command_buffer_, first_binding, binding_count, vbs_vk, offsets_vk);
	}

	void VulkanGraphicsEncoder::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
	{
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y + height;
		viewport.width = width;
		viewport.height = -height;
		viewport.minDepth = min_depth;
		viewport.maxDepth = max_depth;
		vkCmdSetViewport(command_buffer_, 0, 1, &viewport);
	}

	void VulkanGraphicsEncoder::SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height)
	{
		VkRect2D scissor{ {offset_x, offset_y}, {width, height} };
		vkCmdSetScissor(command_buffer_, 0, 1, &scissor);
	}

	void VulkanGraphicsEncoder::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
	{
		vkCmdDraw(command_buffer_, vertex_count, instance_count, first_vertex, first_instance);
	}

	void VulkanGraphicsEncoder::NextSubpass()
	{
		vkCmdNextSubpass(command_buffer_, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanGraphicsEncoder::EndRenderPass()
	{
		vkCmdEndRenderPass(command_buffer_);
	}

	//------------------------------------Transfer Encoder------------------------------------

	void VulkanTransferEncoder::CopyBufferToBuffer(const CopyBufferToBufferDesc& desc)
	{
		assert(desc.dst != nullptr && "fatal:destination buffer is NULL");
		assert(desc.src != nullptr && "fatal:source buffer is NULL");

		rhi::RHIBuffer* src = (rhi::RHIBuffer*)desc.src;
		rhi::RHIBuffer* dst = (rhi::RHIBuffer*)desc.dst;

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = desc.src_offset;
		copyRegion.dstOffset = desc.src_offset;
		copyRegion.size = desc.size;
		vkCmdCopyBuffer(command_buffer_, (VkBuffer)src->GetHandle(), (VkBuffer)dst->GetHandle(), 1, &copyRegion);
	}

	void VulkanTransferEncoder::CopyBufferToImage(RHIBuffer* buffer,
												  RHITexture2D* image,
												  uint32_t              width,
												  uint32_t              height,
												  uint32_t              layer_count)
	{
		assert(buffer != nullptr && "fatal:buffer is NULL");
		assert(image != nullptr && "fatal:image is NULL");

		VulkanTexture2D* image_vk = (VulkanTexture2D*)image;
		VulkanBufferBase* buffer_vk = (VulkanBufferBase*)buffer;

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layer_count;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(command_buffer_, buffer_vk->buffer_, image_vk->image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	//------------------------------------Cmd Buffer----------------------------------------
	void VulkanGraphicsEncoder::ImGui_RenderDrawData(ImDrawData* draw_data)
	{
		ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer_);
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* in_device)
		:device_(in_device)
	{
		

		AllocateCommandBuffers();
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		// Command buffers will be automatically freed when their command pool is destroyed,
		// so we don't need explicit cleanup.
		vkDestroyCommandPool(device_->GetDeviceHandle(), gfx_encoder_.command_pool_, nullptr);
		vkDestroyCommandPool(device_->GetDeviceHandle(), transfer_encoder_.command_pool_, nullptr);
	}

	void VulkanCommandBuffer::AllocateCommandBuffers()
	{
		gfx_encoder_.AllocateCommandBuffer(device_, device_->GetGfxQueue()->GetFamilyIndex());
		transfer_encoder_.AllocateCommandBuffer(device_, device_->GetTransferQueue()->GetFamilyIndex());
	}

	void VulkanCommandBuffer::Begin()
	{
		// Reset Each Frame
		vkResetCommandPool(device_->GetDeviceHandle(), gfx_encoder_.command_pool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		vkResetCommandPool(device_->GetDeviceHandle(), transfer_encoder_.command_pool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		gfx_encoder_.Begin();
		transfer_encoder_.Begin();

		ImGui_ImplVulkan_NewFrame();
	}

	void VulkanCommandBuffer::End()
	{
		gfx_encoder_.End();
		transfer_encoder_.End();
	}
}