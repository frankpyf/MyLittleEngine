#pragma once
#include <Runtime/Utils/BitmaskEnum.h>
#include "Enum.h"

namespace rhi {
	struct DescriptorSetLayout;
	class RenderPass;

	enum class PixelFormat : uint8_t
	{
		Unknown			= 0,
		RGBA8			= 1,
		RGBA32F			= 2,
		DEPTH			= 3
	};
	ENUM_CLASS_FLAGS(PixelFormat)

	enum class SampleType
	{

	};

	enum class TextureUsage : uint8_t
	{
		NONE					= 0x0,
		COLOR_ATTACHMENT		= 0x1,
		DEPTH_ATTACHMENT		= 0x2,
		STENCIL_ATTACHMENT		= 0x4,
		UPLOADABLE				= 0x8,
		SAMPLEABLE				= 0x10,
		TRANSIENT_ATTACHMENT	= 0x20,
		DEFAULT					= UPLOADABLE | SAMPLEABLE
	};
	ENUM_CLASS_FLAGS(TextureUsage)

	struct RHITexture
	{
		uint32_t width;
		uint32_t height;
		uint32_t miplevels = 1;

		PixelFormat format = PixelFormat::RGBA8;
		TextureUsage usage;
	};

	class RHITexture2D
	{
	public:
		struct Descriptor
		{
			uint32_t width;
			uint32_t height;
			uint32_t miplevels = 1;

			PixelFormat format = PixelFormat::RGBA8;
			TextureUsage usage;
		};
		RHITexture2D(uint32_t width, uint32_t height, uint32_t miplevel, PixelFormat in_formt, TextureUsage in_usage)
			:width_(width), height_(height), miplevel_(miplevel), format_(in_formt), usage_(in_usage) {};
		RHITexture2D(std::string_view path, uint32_t miplevel)
			:file_path_(path), width_(0), height_(0),  miplevel_(miplevel) {};
		virtual ~RHITexture2D() = default;
		uint32_t GetWidth() { return width_; };
		uint32_t GetHeight() { return height_; };

		virtual void* GetTextureID() = 0;
		virtual void* GetView() = 0;

		virtual void SetData(const void* data) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		static std::shared_ptr<RHITexture2D> Create(Descriptor desc);
		
	protected:
		uint32_t width_ = 0;
		uint32_t height_ = 0;
		uint32_t miplevel_;

		TextureUsage	usage_;
		PixelFormat		format_;

		std::string file_path_{};
	};

	// ---------------------------------------------------------------------------

	struct RHIBuffer
	{
		struct Descriptor
		{
			uint64_t size;
			// Vertex, Index, Uniform etc.
			ResourceTypes usage;
			// CPU Only, GPU Only, CPU TO GPU, GPU TO CPU   
			MemoryUsage memory_usage;
			// TODO: Add support for dynamic buffer

			bool prefer_device = false;
			bool prefer_host = false;
			bool mapped_at_creation = false;
		};
		uint64_t size = 0;
		uint64_t alignment = 0;
		// Vertex, Index, Uniform etc.
		ResourceTypes usage;

		virtual void SetData(const void* data, uint64_t size) = 0;
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

			// index of the subpass(vulkan only?)
			uint32_t subpass;
		};
		PipelineLayout* layout;
	};
	typedef std::shared_ptr<RHIPipeline> PipelineRef;
}