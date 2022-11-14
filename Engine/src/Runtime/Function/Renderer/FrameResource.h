#pragma once
#include "RenderPass.h"
#include "Runtime/Function/RHI/CommandBuffer.h"

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
		rhi::CommandBuffer* command_buffer;
		// all the render targets one frame needs
		std::vector<RenderTarget*> render_targets;

		//Sync Objects
		rhi::Fence* in_flight_fence;
		rhi::Semaphore* render_finished_semaphore;
		rhi::Semaphore* image_acquired_semaphore;
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

		FrameResource& EndFrame();
	private:
		FrameResource frame_[MAX_FRAMES_IN_FLIGHT];
		uint8_t current_frame = 0;
	};	
}

