#include "mlepch.h"
#include "Nodes.h"
#include "RenderGraph.h"
#include "RenderGraphPass.h"
#include "../FrameResource.h"

namespace renderer {
	PassNode::PassNode(const char* name, RenderGraph& rg, bool is_subpass)
		:DependencyGraph::Node(rg.GetGraph()), pass_name_(name), rg_(rg), is_subpass_(is_subpass)
	{

	}

	void PassNode::SetDependencies(PassNode* dependency)
	{
		if (dependency)
			dependencies_.push_back(dependency->index_);
		else
			dependencies_.push_back(std::numeric_limits<uint32_t>::max());
	}
	//----------------------------------------
	RenderPassNode::RenderPassNode(const char* name, RenderGraph& rg, RenderGraphPassBase* base)
		:PassNode(name, rg), pass_base_(base)
	{
		subpass_graph_ = new RenderGraph();
	}
	RenderPassNode::~RenderPassNode()
	{
		subpass_graph_->Clear();
		delete subpass_graph_;
	}
	void RenderPassNode::Resolve()
	{
		subpass_graph_->Compile();

		uint32_t min_width = std::numeric_limits<uint32_t>::max();
		uint32_t min_height = std::numeric_limits<uint32_t>::max();
		uint32_t max_width = 0;
		uint32_t max_height = 0;

		size_t index = 0;

		for (auto& texture : render_target_.desc_.attachments)
		{
			auto& attachment = pass_base_->desc_.attachments[index++];

			const uint32_t w = texture->desc_.width;
			const uint32_t h = texture->desc_.height;

			min_width  = std::min(min_width, w);
			min_height = std::min(min_height, h);
			max_width  = std::max(max_width, w);
			max_height = std::max(max_height, h);

			attachment.final_layout = texture->last_layout;
		}

		render_target_.desc_.width = min_width;
		render_target_.desc_.height = min_height;
	}

	void RenderPassNode::AddAttachment(ResourceHandle handle, LoadOp load_operation, StoreOp store_operation)
	{
		VirtualResource* resource = rg_.GetResource(handle);
		Resource<RenderGraphTexture>* texture = static_cast<Resource<RenderGraphTexture>*>(resource);

		auto& rp_desc = pass_base_->desc_;

		auto& attachment = rp_desc.attachments.emplace_back();

		rhi::PixelFormat format = texture->resource_.desc_.format;

		attachment.is_depth = format == rhi::PixelFormat::DEPTH;
		attachment.format = format;

		attachment.load_op = load_operation;
		attachment.store_op = store_operation;

		auto& rt_desc = render_target_.desc_;
		// Set up the attachments for framebuffer
		rt_desc.attachments.emplace_back(&texture->resource_);

		// record its index within the framebuffer
		size_t index = rt_desc.attachments.size() - 1;
		declared_resources_.emplace(handle, index);
	}

	void RenderPassNode::RegisterResource(size_t handle, Usage usage)
	{
		VirtualResource* resource = rg_.GetResource(handle);
		Resource<RenderGraphTexture>* texture = static_cast<Resource<RenderGraphTexture>*>(resource);
		auto& texture_desc = texture->resource_.desc_;
		resource->NeedByPass(this);

		switch (usage)
		{
		case DEFAULT_R_USAGE:
			texture_desc.usage |= rhi::TextureUsage::SAMPLEABLE;
			break;
		case DEFAULT_W_USAGE:
			size_t index = declared_resources_.at(handle);
			auto& attachment = pass_base_->desc_.attachments[index];
			attachment.initial_layout = texture->resource_.last_layout;

			if (texture->resource_.desc_.format == rhi::PixelFormat::DEPTH)
			{
				texture_desc.usage |= rhi::TextureUsage::DEPTH_ATTACHMENT;
			}
			else
			{
				texture_desc.usage |= rhi::TextureUsage::COLOR_ATTACHMENT;
			}
			break;
		}
	}

	void RenderPassNode::Instantiate()
	{
		pass_base_->Instantiate();

		for (auto& desc : pipelines_)
		{
			desc.render_pass = pass_base_->actual_rp_;
			pass_base_->actual_rp_->CreatePipeline(desc);
		}

		render_target_.desc_.pass = pass_base_->actual_rp_;
	}

	void RenderPassNode::Execute(FrameResource& resource)
	{
		render_target_.Create();

		pass_base_->Exec(*render_target_.render_target, resource);

		render_target_.Destroy(resource);
	}

	void RenderPassNode::AssembleRenderTarget()
	{
		
	}

	void RenderPassNode::UpdateAttachmentLayout(ResourceHandle handle)
	{
		size_t index = declared_resources_.at(handle);
		auto& attachment = pass_base_->desc_.attachments[index];
		attachment.final_layout = rhi::ImageLayout::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	//-----------------------------------------

	void SubpassNode::Resolve()
	{
		for (auto& dependency : dependencies_)
		{
			subpass_desc_.dependencies.emplace_back(dependency);
		}
		parent_->pass_base_->desc_.subpasses.emplace_back(subpass_desc_);
	}

	void SubpassNode::RegisterResource(size_t handle, Usage usage)
	{
		VirtualResource* resource = parent_->rg_.GetResource(handle);
		Resource<RenderGraphTexture>* texture = static_cast<Resource<RenderGraphTexture>*>(resource);

		size_t attachment = parent_->declared_resources_.at(handle);

		switch (usage)
		{
		case DEFAULT_R_USAGE: 
			texture->resource_.last_layout = rhi::ImageLayout::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			subpass_desc_.input_attachments.push_back(attachment);
			break;
		case DEFAULT_W_USAGE:
			if (texture->resource_.desc_.format == rhi::PixelFormat::DEPTH)
			{
				texture->resource_.last_layout = rhi::ImageLayout::IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				subpass_desc_.use_depth_stencil = true;
			}
			else
			{
				texture->resource_.last_layout = rhi::ImageLayout::IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				subpass_desc_.color_attachments.push_back(attachment);
			}
			break;
		}
	}
	//-----------------------------------------

	ResourceNode::ResourceNode(RenderGraph& rg, size_t resource_handle)
		:DependencyGraph::Node(rg.GetGraph()),rg_(rg), resource_index_(resource_handle)
	{

	}
	void ResourceNode::SetIncomingEdge(Edge* edge)
	{
		incoming_edge_ = edge;
	}

	void ResourceNode::SetOutgoingEdge(Edge* edge)
	{
		outgoing_edges_.emplace_back(edge);
	}

	char const* ResourceNode::GetName() const noexcept
	{
		return rg_.GetResource(resource_index_)->name_;
	}

	VirtualResource* ResourceNode::GetResource()
	{
		return rg_.GetResource(resource_index_);
	}

	PassNode* ResourceNode::GetWriterPass()
	{
		auto& graph = rg_.GetGraph();
		if (incoming_edge_)
		{
			return static_cast<PassNode*>(graph.GetNode(incoming_edge_->from));
		}
		return nullptr;
	}
}