#pragma once
#include "Runtime/Core/Base/Singleton.h"
#include "Runtime/Function/RHI/RHICommands.h"
#include "FrameResource.h"
#include "RenderGraph/RenderGraph.h"

namespace resource {
	struct Vertex;
}

namespace renderer {
	class Renderer : public engine::Singleton<Renderer>
	{
		friend class RenderGraph;
	public:
		Renderer();
		virtual ~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void Begin();
		void Tick(float time_step);
		void End();

		void Init();

		void Shutdown();

		RenderGraph& GetRenderGraph() { return render_graph_; };

		// TEMP Functions
		rhi::BufferRef LoadModel(const std::vector<resource::Vertex>& in_vertices);
		rhi::BufferRef LoadIndex(const std::vector<uint16_t>& in_indecies);
		void LoadAllocator(rhi::DescriptorAllocator* desc_allocator)
		{
			desc_allocator_ = desc_allocator;
		}
		void LoadLayout(rhi::DescriptorSetLayout* layout)
		{
			global_layout_ = layout;
		}

		FrameResource& GetCurrentFrame()
		{
			return frames_manager_.GetCurrentFrame();
		};
	private:
		RenderGraph render_graph_;
		FrameResourceMngr frames_manager_;
		bool is_frame_started_{ false };

		rhi::DescriptorAllocator* desc_allocator_;
		rhi::DescriptorSetLayout* global_layout_;
	};
}

