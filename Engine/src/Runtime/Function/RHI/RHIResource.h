#pragma once
#include "Enum.h"

namespace rhi {
	struct DescriptorSetLayout;
	class RenderPass;

	struct RHITexture
	{
		struct Descriptor
		{
			uint32_t width;
			uint32_t height;
			uint32_t depth;
			uint32_t miplevels = 1;

			uint32_t array_layers = 1;

			PixelFormat format = PixelFormat::RGBA8;
			TextureUsage usage;
		};
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 0;
		uint32_t miplevels = 0;

		uint32_t layers_count;

		TextureUsage	usage;
		PixelFormat		format;

		//For use of ImGui
		void* texture_id = nullptr;
		virtual void RegisterForImGui() {};
	};
	typedef std::shared_ptr<RHITexture> TextureRef;

	struct RHISampler
	{
	};
	typedef std::shared_ptr<RHISampler> SamplerRef;

	// ---------------------------------------------------------------------------

	struct RHIBuffer
	{
		struct Descriptor
		{
			// Vertex, Index, Uniform etc.
			ResourceTypes usage;
			// CPU Only, GPU Only, CPU TO GPU, GPU TO CPU   
			MemoryUsage memory_usage;

			// support for dynamic buffer

			// Set to 1 if it is not a dynamic buffer
			uint32_t element_count = 1;
			// actual size allocated to each element
			uint32_t element_stride;

			bool prefer_device = false;
			bool prefer_host = false;
			bool mapped_at_creation = false;
		};
		uint64_t size = 0;
		uint32_t alignment = 0;
		// Vertex, Index, Uniform etc.
		ResourceTypes usage;

		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;
	};
	typedef std::shared_ptr<RHIBuffer> BufferRef;

	// ---------------------------------------------------------------------------

	struct ShaderModule
	{
	};

	// Root Signature in d3d
	struct PipelineLayout
	{
		struct Descriptor
		{
			uint32_t set_layout_count = 0;
			DescriptorSetLayout** layouts;
			uint32_t push_constant_count = 0;
		};
	};

	struct RHIPipeline
	{
		struct Descriptor
		{
			PrimitiveTopology topology;

			RenderPass* render_pass = nullptr;

			PipelineLayout* layout = nullptr;

			ShaderModule* vert_shader;
			ShaderModule* frag_shader;

			// TEMP
			bool use_vertex_attribute = true;

			// index of the subpass(vulkan only?)
			uint32_t subpass;
		};
		PipelineLayout* layout;
	};
	typedef std::shared_ptr<RHIPipeline> PipelineRef;
}