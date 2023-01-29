#pragma once
#include "DependencyGraph.h"
#include "Runtime/Function/RHI/RenderPass.h"
#include "RenderGraphPass.h"
#include "VirtualResource.h"

#define MULTIPASS_ENABLED

namespace renderer {
	class RenderGraph;
	class SubpassGraph;
	struct FrameResource;
	using ResourceHandle = size_t;

	using Usage = rhi::TextureUsage;
	static constexpr Usage DEFAULT_R_USAGE = Usage::SAMPLEABLE;
	static constexpr Usage DEFAULT_W_USAGE = Usage::COLOR_ATTACHMENT;

	class PassNode : public DependencyGraph::Node
	{
		friend class RenderGraph;
	public:
		PassNode(const char* name, RenderGraph& rg, bool is_subpass = false);
		~PassNode() override = default;

		virtual char const* GetName() const noexcept override { return pass_name_; };

		virtual void RegisterResource(ResourceHandle handle, Usage usage) = 0;

		virtual void Instantiate() {};
		virtual void Execute(FrameResource& resource) {};
		virtual void Resolve() {};
		virtual void UpdateAttachmentLayout(ResourceHandle handle) {};

		void SetDependencies(PassNode* dependency);


		// index in the render pass path, set during compile
		size_t index_;

		std::vector<size_t> dependencies_;

		bool is_subpass_ = false;
	protected:
		RenderGraph& rg_;
		const char* pass_name_ = nullptr;

		// resources that this pass needs to devirtualize
		std::unordered_set<VirtualResource*> devirtualize_;
		// resources that this pass needs to destroy
		std::unordered_set<VirtualResource*> destroy_;
	};

	class SubpassNode : public PassNode
	{
		friend class RenderGraph;
		friend class RenderPassNode;
	public:
		SubpassNode(const char* name, RenderGraph& rg, RenderPassNode* parent, uint32_t index)
			:PassNode(name, rg, true), parent_(parent), subpass_index_(index) {};

		virtual void RegisterResource(ResourceHandle handle, Usage usage) override;
		virtual void Resolve() override;

		RenderPassNode*	parent_ = nullptr;

		uint32_t	subpass_index_;
	private:
		rhi::RenderPass::Subpass subpass_desc_;
	};

	class RenderPassNode : public PassNode
	{
		friend class RenderGraph;
		friend class SubpassNode;
	public:
		RenderPassNode(const char* name, RenderGraph& rg, RenderGraphPassBase* base);
		~RenderPassNode() override;

		using LoadOp = rhi::RenderPass::AttachmentDesc::LoadOp;
		using StoreOp = rhi::RenderPass::AttachmentDesc::StoreOp;
		// Add Attachment
		void AddAttachment(ResourceHandle handle, LoadOp load_operation, StoreOp store_operation);

		virtual void RegisterResource(ResourceHandle handle, Usage usage) override;

		virtual void Instantiate() override;
		virtual void Execute(FrameResource& resource) override;
		virtual void Resolve() override;
		virtual void UpdateAttachmentLayout(ResourceHandle handle) override;

		virtual void AssembleRenderTarget();

		// Used to manage subpasses
		RenderGraph* subpass_graph_;

		std::unordered_map<ResourceHandle, size_t> declared_resources_;
	protected:
		std::unique_ptr<RenderGraphPassBase>		pass_base_;
		RenderGraphRenderTarget	render_target_;

		std::vector<rhi::RHIPipeline::Descriptor> pipelines_;
		std::vector<rhi::DescriptorSet*> descriptor_sets_;
	};

	class PresentPassNode :public RenderPassNode
	{
	public:
		PresentPassNode(const char* name, RenderGraph& rg, RenderGraphPassBase* base)
			:RenderPassNode(name, rg, base) 
		{
			pass_base_->desc_.is_for_present = true;
		};
		//virtual void AssembleRenderTarget() override;
	};

	//--------------------------------------------------------
	class ResourceNode : public DependencyGraph::Node
	{
		friend class RenderGraph;
		using Edge = DependencyGraph::Edge;
	public:
		ResourceNode(RenderGraph& rg, size_t resource_handle);
		void SetIncomingEdge(Edge* edge);
		void SetOutgoingEdge(Edge* edge);

		PassNode* GetWriterPass();

		virtual char const* GetName() const noexcept override;
		VirtualResource* GetResource();

	private:
		RenderGraph& rg_;

		size_t resource_index_;

		std::vector<Edge*>	outgoing_edges_;
		Edge*				incoming_edge_ = nullptr;
	};
}
