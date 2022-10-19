#pragma once
#include "Runtime/Core/Base/Singleton.h"
#include <glm/glm.hpp>

namespace renderer {
	struct SubpassDesc
	{
		bool				use_depth_stencil = true;
		// index of the color attachments in the framebuffer
		std::vector<uint32_t>			color_attachments;
		// index of the input attachments in the framebuffer
		std::vector<uint32_t>	input_attachments;
	};	
	
	enum class AttachmentFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	enum class LoadOp
	{
		LOAD = 0,
		STORE = 1,
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
		AttachmentFormat format;
		LoadOp load_op;
		StoreOp store_op;
		ImageLayout initial_layout = ImageLayout::IMAGE_LAYOUT_UNDEFINED;
		ImageLayout final_layout;
	};

	struct ColorAttachmentDesc : AttachmentDesc
	{
		AttachmentFormat format = AttachmentFormat::RGBA8;
		ImageLayout final_layout = ImageLayout::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	};

	struct DepthStencilAttachmentDesc : AttachmentDesc
	{
		AttachmentFormat format = AttachmentFormat::DEPTH24STENCIL8;
		ImageLayout final_layout = ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	};

	struct RenderPassDesc
	{
		std::vector<ColorAttachmentDesc>	color_attachments;
		DepthStencilAttachmentDesc			depth_stencil_attachment;
		bool								is_for_present = false;

		uint32_t							width = 0;
		uint32_t							height = 0;
		std::vector<SubpassDesc>			subpasses;
	};

	class RenderPass
	{
	public:
		RenderPass(const char* render_pass_name,
			const std::function<void(RenderPass*)>& setup,
			const std::function<void(RenderPass*)>& exec)
			: pass_name_(render_pass_name), setup_func_(setup), exec_func_(exec) {};
		virtual ~RenderPass() = default;
		const std::string& GetName() const { return pass_name_; }
		void Setup() { setup_func_(this); };
		void Exec() { exec_func_(this); };

		virtual void* GetHandle() = 0;
		virtual void* GetFramebuffer(uint32_t index) = 0;
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;

		static RenderPass* Create(const char* render_pass_name, const RenderPassDesc& desc,
												  const std::function<void(RenderPass*)>& setup,
												  const std::function<void(RenderPass*)>& exec);
	protected:
		std::string pass_name_;

		std::function<void(RenderPass*)> setup_func_;
		std::function<void(RenderPass*)> exec_func_;
	};

	class RenderGraph : public engine::Singleton<RenderGraph>
	{
	public:
		virtual ~RenderGraph() = default;
		void AddRenderPass(const char* render_pass_name, const RenderPassDesc& desc,
			const std::function<void(RenderPass*)>& setup,
			const std::function<void(RenderPass*)>& exec);

		void RemoveRenderPass(const char* render_pass_name);

		void Clear();

		void Run();

		void Compile();

		void Setup();
	private:

		std::list<RenderPass*> render_passes_{};
		std::list<RenderPass*> render_pass_path_{};

		bool is_compiled_ = false;
	};
}

