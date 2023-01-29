#pragma once
#include "Runtime/Function/RHI/RenderPass.h"
#include "Runtime/Function/RHI/CommandBuffer.h"
#include "Runtime/Function/RHI/RHIResource.h"
#include "Runtime/Function/RHI/Descriptor.h"

namespace rhi {
	class RHI;
	struct Semaphore;
	struct Fence;
}
namespace renderer {
	/// <summary>
	/// Each Frame has its own command buffer, command pool and render targets etc.
	/// </summary>
	struct FrameResource
	{
		// graphics, compute and transfer 
		rhi::CommandBuffer* command_buffer = nullptr;

		//Sync Objects
		rhi::Fence* in_flight_fence = nullptr;
		rhi::Semaphore* render_finished_semaphore = nullptr;
		rhi::Semaphore* image_acquired_semaphore = nullptr;

		std::vector<std::shared_ptr<rhi::RHITexture2D>> texture_dump;
		std::vector<std::shared_ptr<rhi::RenderTarget>> render_target_dump;
	};

	class FrameResourceMngr
	{
	public:
		static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 3;
		virtual ~FrameResourceMngr() = default;

		void CreateFrames();
		void DestroyFrames();
		FrameResource& BeginFrame();
		inline FrameResource& GetCurrentFrame() { return frame_[current_frame]; };
		inline uint8_t GetFrameIndex() { return current_frame; };

		FrameResource& EndFrame();

		void Clean();
	private:
		FrameResource frame_[MAX_FRAMES_IN_FLIGHT];
		uint8_t current_frame = 0;

	};	
}

