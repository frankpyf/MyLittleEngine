#pragma once
#include "Runtime/Function/RHI/RHIResource.h"
#include "Runtime/Function/RHI/RenderPass.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Function/Renderer/FrameResource.h"

namespace renderer {
	class PassNode;
	class ResourceNode;
	class DependencyGraph;

	class VirtualResource {
	public:
		VirtualResource(const char* name, bool imported = false)
			:name_(name), is_imported_(imported) {};
		virtual ~VirtualResource() = default;

		virtual void Instantiate() = 0;
		virtual void Destroy(FrameResource& frame) = 0;

		virtual void Connect(DependencyGraph& dg, ResourceNode* resource, PassNode* pass);
		virtual void Connect(DependencyGraph& dg, PassNode* pass, ResourceNode* resource);
		
		void NeedByPass(PassNode* node);

		const char* const name_;

		uint32_t ref_count_ = 0;

		// first pass that needs this resource, so it has the responsibility to instantiate the resource
		PassNode* first_ = nullptr;
		// last pass that needs this resource, so it has the resonsibility to destroy the resource
		PassNode* last_ = nullptr;

		const bool is_imported_;
	};

	template<typename T>
	class Resource : public VirtualResource
	{
	public:
		using Descriptor = typename T::Descriptor;

		Resource(const char* name, Descriptor const& desc)
			:VirtualResource(name)
		{
			resource_.desc_ = desc;
		}
		Resource(const char* name, Descriptor const& desc, const T& in_resource)
			:VirtualResource(name, true), resource_(in_resource)
		{
			resource_.desc_ = desc;
		}
	
		virtual void Instantiate() override
		{
			if (!is_imported_)
			{
				resource_.Create();
			}
		}

		virtual void Destroy(FrameResource& frame) override
		{
			if (!is_imported_)
			{
				resource_.Destroy(frame);
			}
		}
		T resource_{};
	};

	struct RenderGraphTexture
	{
		rhi::TextureRef texture;
		using Descriptor = rhi::RHITexture::Descriptor;
		Descriptor desc_{};

		rhi::ImageLayout last_layout = rhi::ImageLayout::IMAGE_LAYOUT_UNDEFINED;

		void Create() 
		{
			texture = rhi::RHI::GetRHIInstance().RHICreateTexture(desc_);
		};
		void Destroy(FrameResource& frame) 
		{
			frame.texture_dump.push_back(texture);
		};
	};

	struct RenderGraphRenderTarget
	{
		struct Descriptor {
			uint32_t width;
			uint32_t height;

			std::vector<RenderGraphTexture*> attachments;

			rhi::RenderPass* pass;
		};

		std::shared_ptr<rhi::RenderTarget> render_target;
		
		Descriptor desc_{};
		void Create() 
		{
			rhi::RenderTarget::Descriptor desc{};
			for (auto attachment : desc_.attachments)
			{
				desc.attachments.push_back(attachment->texture.get());
				// TEMP: Force to match the texture size
				attachment->desc_.width = desc_.width;
				attachment->desc_.height = desc_.height;
			}
			desc.width = desc_.width;
			desc.height = desc_.height;
			desc.pass = desc_.pass;
			render_target = rhi::RenderTarget::Create(desc);
		};
		void Destroy(FrameResource& frame) 
		{
			auto& dump = frame.render_target_dump.emplace_back();
			dump = std::move(render_target);
		};
	};
}