#pragma once
#include "RHI.h"

namespace rhi {
	class RHICommands
	{
	public:
		static void Init()
		{
			rhi_.Init();
		};
		static void Begin()
		{
			rhi_.Begin();
		};
		static void BeginRenderPass(renderer::RenderPass& pass)
		{
			rhi_.BeginRenderPass(pass);
		};
		static void SetViewport(float x, float y, float width, float height, float min_depth = 0.0f, float max_depth = 1.0f) 
		{
			rhi_.SetViewport(x, y, width, height, min_depth, max_depth);
		};
		static void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height)
		{
			rhi_.SetScissor(offset_x, offset_y, width, height);
		};
		static void GfxQueueSubmit()
		{
			rhi_.GfxQueueSubmit();
		}
		
		static void EndRenderPass()
		{
			rhi_.EndRenderPass();
		};
		static void End()
		{
			rhi_.End();
		}

		// Other commands
		static void ImGui_ImplMLE_RenderDrawData(ImDrawData* draw_data)
		{
			rhi_.ImGui_ImplMLE_RenderDrawData(draw_data);
		}
		static void* GetCurrentFrame()
		{
			return rhi_.GetCurrentFrame();
		}
		static void* GetRHIInstance()
		{
			return rhi_.GetNativeInstance();
		}
		static void* GetGfxQueue()
		{
			return rhi_.GetNativeGraphicsQueue();
		}
		static uint32_t GetGfxQueueFamilyIndex()
		{
			return rhi_.GetGfxQueueFamily();
		}
		static void* GetPhysicalDevice()
		{
			return rhi_.GetNativePhysicalDevice();
		}
		static void Shutdown()
		{
			rhi_.Shutdown();
		}

	private:
		static RHI& rhi_;
	};
}

