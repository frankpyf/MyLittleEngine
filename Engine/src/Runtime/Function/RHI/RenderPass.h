#pragma once
#include "Runtime/Core/Base/Singleton.h"
#include "RHIResource.h"
#include <glm/glm.hpp>

namespace renderer {
	struct FrameResource;
}

namespace rhi {
	// forward declaration
	class Pipeline;
	struct PipelineDesc;
	class CommandBuffer;
	class RenderPass;
	struct RHIPipeline;
	typedef std::shared_ptr<RHIPipeline> PipelineRef;
	
	enum class ImageLayout
	{
		IMAGE_LAYOUT_UNDEFINED = 0,
		IMAGE_LAYOUT_GENERAL = 1,
		IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
		IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
		IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
		IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
		IMAGE_LAYOUT_PRESENT = 6
	};

	
	class RenderTarget
	{
	public:
		struct Descriptor
		{
			std::vector<rhi::RHITexture2D*> attachments;

			uint32_t width;
			uint32_t height;
			glm::fvec4 clear_value = { 0.45f, 0.55f, 0.60f, 1.00f };

			RenderPass* pass;
		};

		friend class RenderPass;
		RenderTarget(uint32_t width, uint32_t height, glm::fvec4 clear_color)
			:width_(width), height_(height), clear_value_(clear_color) {};
		virtual ~RenderTarget() = default;
		virtual void* GetHandle() = 0;
		inline glm::fvec4 GetClearColor() { return clear_value_; };
		inline uint32_t GetWidth() { return width_; };
		inline uint32_t GetHeight() { return height_; };

		static std::unique_ptr<RenderTarget> Create(const RenderTarget::Descriptor& desc);
	protected:
		glm::fvec4 clear_value_;
		uint32_t width_;
		uint32_t height_;
	};

	class RenderPass
	{
	public:
		//For vulkan only
		struct Subpass
		{
			// index of the color attachments in the framebuffer
			std::vector<uint32_t>			color_attachments;
			// index of the input attachments in the framebuffer
			std::vector<uint32_t>			input_attachments;

			std::vector<size_t>				dependencies;

			bool							use_depth_stencil = false;
		};

		struct AttachmentDesc
		{
			enum class LoadOp
			{
				LOAD = 0,
				CLEAR = 1,
				DONT_CARE = 2
			};

			enum class StoreOp
			{
				STORE = 0,
				DONT_CARE = 1
			};

			bool is_depth = false;

			LoadOp load_op = LoadOp::CLEAR;
			StoreOp store_op = StoreOp::STORE;

			PixelFormat format = rhi::PixelFormat::RGBA8;
			
			ImageLayout initial_layout = ImageLayout::IMAGE_LAYOUT_UNDEFINED;
			ImageLayout final_layout = ImageLayout::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		};

		struct Descriptor
		{
			std::vector<AttachmentDesc>	attachments;

			// subpass seems to be vulkan only
			std::vector<RenderPass::Subpass>			subpasses;

			bool								is_for_present = false;
		};

	
		RenderPass(bool is_for_present)
			: is_for_present_(is_for_present){};
		virtual ~RenderPass();

		virtual void* GetHandle() = 0;

		PipelineRef CreatePipeline(const RHIPipeline::Descriptor& desc);
		PipelineRef GetPipeline(uint32_t index) { return pipelines_[index]; };

		static RenderPass* Create(const Descriptor& desc);

		bool is_for_present_ = false;
	protected:
		std::vector<PipelineRef> pipelines_{};
	};
}

