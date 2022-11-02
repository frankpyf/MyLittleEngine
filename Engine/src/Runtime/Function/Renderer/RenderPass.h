#pragma once
#include "Runtime/Core/Base/Singleton.h"
#include <glm/glm.hpp>
namespace rhi {
	class RHITexture2D;
}

namespace renderer {
	// forward declaration
	class Pipeline;
	struct PipelineDesc;

	struct RenderTargetDesc
	{
		std::vector<uint32_t> attachments_index;

		std::vector<void*> attachments;

		uint32_t width;
		uint32_t height;
		glm::fvec4 clear_value = { 0.0f,0.0f,0.0f,1.0f };
	};

	class RenderTarget
	{
	public:
		friend class RenderPass;
		RenderTarget(uint32_t width, uint32_t height, glm::fvec4 clear_color)
			:width_(width), height_(height), clear_value_(clear_color) {};
		virtual ~RenderTarget() = default;
		virtual void* GetHandle() = 0;
		inline glm::fvec4 GetClearColor() { return clear_value_; };
		inline uint32_t GetWidth() { return width_; };
		inline uint32_t GetHeight() { return height_; };

		static RenderTarget* Create(RenderPass& pass);
	protected:
		glm::fvec4 clear_value_ = { 0.0f,0.0f,0.0f,1.0f };
		uint32_t width_;
		uint32_t height_;
	};

	struct SubpassDesc
	{
		bool				use_depth_stencil = true;
		// index of the color attachments in the framebuffer
		std::vector<uint32_t>			color_attachments;
		// index of the input attachments in the framebuffer
		std::vector<uint32_t>			input_attachments;
	};	
	
	enum class AttachmentFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,
		SWAPCHAIN_FORMAT,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

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

	struct AttachmentDesc
	{
		LoadOp load_op;
		StoreOp store_op;
		ImageLayout initial_layout = ImageLayout::IMAGE_LAYOUT_UNDEFINED;
	};

	struct ColorAttachmentDesc : AttachmentDesc
	{
		AttachmentFormat format = AttachmentFormat::RGBA8;
		ImageLayout final_layout = ImageLayout::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	};

	struct DepthStencilAttachmentDesc : AttachmentDesc
	{
		AttachmentFormat format = AttachmentFormat::DEPTH24STENCIL8;
		ImageLayout final_layout = ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	};

	struct RenderPassDesc
	{
		std::vector<ColorAttachmentDesc>	color_attachments;
		DepthStencilAttachmentDesc*			depth_stencil_attachment = nullptr;

		// subpass seems to be vulkan only
		std::vector<SubpassDesc>			subpasses;

		bool								is_for_present = false;
	};

	class RenderPass
	{
	public:
		RenderPass(const char* render_pass_name, bool is_for_present,
			void(*exec)(RenderPass& rp, RenderTarget& rt))
			: pass_name_(render_pass_name), is_for_present_(is_for_present), exec_func_(exec) {};
		virtual ~RenderPass();
		const std::string& GetName() const { return pass_name_; }

		virtual void* GetHandle() = 0;
		RenderTargetDesc& GetRTDesc() { return rt_desc_; };
		Pipeline* CreatePipeline(const char* vert_path,
								 const char* frag_path,
								 const PipelineDesc& desc);
		Pipeline* GetPipeline(uint32_t index) { return pipelines_[index]; };

		static RenderPass* Create(const char* render_pass_name, const RenderPassDesc& desc,
								  void(*exec)(RenderPass& rp, RenderTarget& rt));

		void(*exec_func_)(RenderPass& rp, RenderTarget& rt);

		bool is_for_present_ = false;
	protected:
		std::string pass_name_;

		RenderTargetDesc rt_desc_;

		std::vector<Pipeline*> pipelines_{};
	};

	class RenderGraph : public engine::Singleton<RenderGraph>
	{
	public:
		virtual ~RenderGraph() = default;

		/// <summary>
		/// Add a render pass to the render graph
		/// </summary>
		/// <typeparam name="Setup">Lambda</typeparam>
		/// <typeparam name="Execute">Lambda</typeparam>
		/// <param name="render_pass_name">the name of this render pass</param>
		/// <param name="desc">description about the render pass, see: RenderPassDesc</param>
		/// <param name="setup">setup lambda, delcare which and how resources are used by this pass</param>
		/// <param name="exec">execute lambda</param>
		template <typename Setup>
		void AddRenderPass(const char* render_pass_name, const RenderPassDesc& desc,
			               const Setup& setup,
						   void(*exec)(RenderPass& rp, RenderTarget& rt))
		{
			auto rp = RenderPass::Create(render_pass_name, desc, exec);
			setup(rp, rp->GetRTDesc());
			render_passes_.emplace_back(rp);

			is_compiled_ = false;
		}

		RenderPass& GetRenderPass(const char* render_pass_name);

		void RemoveRenderPass(const char* render_pass_name);

		void Clear();

		void Run();

		void PostRun();

		void Compile();

		uint32_t RegisterTransientResource();
		uint32_t RegisterResource(rhi::RHITexture2D* resource);

		void AssembleRenderTarget(RenderTarget& rt);
	private:
		std::list<RenderPass*> render_passes_{};
		std::list<RenderPass*> render_pass_path_{};
		std::vector<rhi::RHITexture2D*> resources_{};
		std::vector<RenderTarget*> rts_;

		bool is_compiled_ = false;
	};
}

