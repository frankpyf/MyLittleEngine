#include "mlepch.h"
#include "FrameResource.h"
#include "Runtime/Function/RHI/RHI.h"

namespace renderer {
	void FrameResourceMngr::CreateFrames()
	{
		for (uint8_t index = 0; index < MAX_FRAMES_IN_FLIGHT; index++)
		{
			frame_[index].command_buffer_ = rhi::CommandBuffer::Create();
		}
	}

	void FrameResourceMngr::DestroyFrames()
	{
		for (uint8_t index = 0; index < MAX_FRAMES_IN_FLIGHT; index++)
		{
			if (frame_[index].command_buffer_)
				delete frame_[index].command_buffer_;
			for (auto& rt : frame_[index].render_targets_)
			{
				if (rt)
					delete rt;
			}
			frame_[index].render_targets_.clear();
		}
	}

	FrameResource& FrameResourceMngr::BeginFrame()
	{
		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

		frame_[current_frame].command_buffer_->Begin();
		// Since we need to recreate rts each frame, so ...
		for (auto& rt : frame_[current_frame].render_targets_)
		{
			delete rt;
		}
		// reserve the capability though
		frame_[current_frame].render_targets_.clear();

		return frame_[current_frame];
	}

	FrameResource& FrameResourceMngr::EndFrame()
	{
		frame_[current_frame].command_buffer_->End();
		return frame_[current_frame];
	}
}