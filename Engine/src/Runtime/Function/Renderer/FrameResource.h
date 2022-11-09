#pragma once
#include "RenderPass.h"
#include "Runtime/Function/RHI/CommandBuffer.h"

namespace rhi {
	class RHI;
}
namespace renderer {
	/// <summary>
	/// Each Frame has its own command buffer, command pool and render targets etc.
	/// </summary>
	struct FrameResource
	{
		// graphics, compute and transfer 
		rhi::CommandBuffer* command_buffer_;
		// all the render targets one frame needs
		std::vector<RenderTarget*> render_targets_;
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

