#pragma once

namespace renderer
{
	class RenderPass;
	
	struct PipelineDesc
	{
		enum class PrimitiveTopology
		{
			PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
			PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
			PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
			PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
			PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
			PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
		} topology;

		RenderPass* render_pass = nullptr;

		// void* pipeline_layout = nullptr;

		// index of the subpass(vulkan only?)
		uint32_t subpass;
	};
	class Pipeline
	{
	public:
		virtual ~Pipeline() = default;
		virtual void* GetHandle() = 0;
		static Pipeline* Create(const char* vert_path,
								const char* frag_path,
								const PipelineDesc& desc);
	};
}