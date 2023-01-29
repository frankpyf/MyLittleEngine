#include "mlepch.h"
#include "RenderGraph.h"
#include "VirtualResource.h"
#include "../Renderer.h"

namespace renderer {
	RenderGraph::SubpassBuilder& RenderGraph::SubpassBuilder::Read(ResourceHandle resource)
	{
		rg_.Read(subpass_node_, resource);

		return *this;
	}
	RenderGraph::SubpassBuilder& RenderGraph::SubpassBuilder::Write(ResourceHandle resource)
	{
		rg_.Write(subpass_node_, resource);
		
		return *this;
	}
	RenderGraph::SubpassBuilder& RenderGraph::SubpassBuilder::SetPipeline(const rhi::RHIPipeline::Descriptor& desc)
	{
		subpass_node_->parent_->rg_.SetPipelineInternal(subpass_node_, desc);
		return *this;
	}
	// --------------------------------------------------------------
	RenderGraph::Builder& RenderGraph::Builder::Read(uint32_t set, uint32_t binding, ResourceHandle resource)
	{
		rg_.Read(node_, resource);

		return *this;
	}
	RenderGraph::Builder& RenderGraph::Builder::Read(ResourceHandle resource)
	{
		rg_.Read(node_, resource);

		return *this;
	}
	RenderGraph::Builder& RenderGraph::Builder::ReadWrite(uint32_t set, uint32_t binding, ResourceHandle resource, LoadOp load_operation, StoreOp store_operation)
	{
		rg_.Read(node_, resource);

		rg_.Write(node_, resource);

		RenderPassNode* rp_node = static_cast<RenderPassNode*>(node_);
		rp_node->AddAttachment(resource, load_operation, store_operation);

		return *this;
	}
	RenderGraph::Builder& RenderGraph::Builder::Write(ResourceHandle resource, LoadOp load_operation, StoreOp store_operation)
	{
		rg_.Write(node_, resource);

		RenderPassNode* rp_node = static_cast<RenderPassNode*>(node_);
		rp_node->AddAttachment(resource, load_operation, store_operation);

		return *this;
	}

	RenderGraph::Builder& RenderGraph::Builder::SetPipeline(const rhi::RHIPipeline::Descriptor& desc)
	{
		rg_.SetPipelineInternal(node_, desc);
		return *this;
	}

	//-----------------------------------------------------------------
	RenderGraph::Builder RenderGraph::AddPassInternal(const char* name, RenderGraphPassBase* base)
	{
		PassNode* node = new RenderPassNode(name, *this, base);
		base->SetNode(node);
		pass_nodes_.push_back(node);

		return { *this,node };
	}

	RenderGraph::SubpassBuilder RenderGraph::AddSubPassInternal(const char* name, RenderPassNode* parent)
	{
		SubpassNode* node = new SubpassNode(name, *this, parent, pass_nodes_.size());
		pass_nodes_.push_back(node);
		node->DontCull();

		return { *this,node };
	}

	PassNode& RenderGraph::GetRenderPass(const char* render_pass_name)
	{
		for (auto& pass : pass_nodes_)
		{
			if (pass->pass_name_ == render_pass_name)
			{
				return *pass;
			}
		}
	}

	void RenderGraph::RemoveRenderPass(const char* render_pass_name)
	{
		pass_nodes_.erase(std::remove_if(pass_nodes_.begin(), pass_nodes_.end(), [render_pass_name](auto* rp) {
			return rp->pass_name_ == render_pass_name;
			}));

		is_compiled_ = false;
		render_pass_path_.clear();
	}

	void RenderGraph::Clear()
	{
		graph_.Clear();
		for (auto& rp : pass_nodes_)
		{
			delete rp;
		}
		for (auto& resource_node : resource_nodes_)
		{
			delete resource_node;
		}
		for (auto& resource : resources_)
		{
			delete resource;
		}
		pass_nodes_.clear();
		render_pass_path_.clear();
		resources_.clear();
		resource_nodes_.clear();
	}

	void RenderGraph::Compile()
	{
		render_pass_path_.clear();
		// Cull unused nodes first
		graph_.Cull();

		// copy the used pass nodes to render pass path
		auto it = std::stable_partition(pass_nodes_.begin(), pass_nodes_.end(), [](auto const& pass_node) {
			return !pass_node->IsCulled(); });
		render_pass_path_.resize(it - pass_nodes_.begin());
		std::copy(std::begin(pass_nodes_), it, render_pass_path_.begin());

		size_t index = 0;
		for (auto pass : render_pass_path_)
		{
			pass->index_ = index++;
			auto const& reads = graph_.GetIncomingEdges(pass);
			for (auto const& edge : reads)
			{
				auto resource_node = static_cast<ResourceNode*>(graph_.GetNode(edge->from));
				pass->RegisterResource(resource_node->resource_index_, DEFAULT_R_USAGE);
				pass->SetDependencies(resource_node->GetWriterPass());
			}

			auto const& writes = graph_.GetOutgoingEdges(pass);
			for (auto const& edge : writes)
			{
				auto resource_node = static_cast<ResourceNode*>(graph_.GetNode(edge->to));
				pass->RegisterResource(resource_node->resource_index_, DEFAULT_W_USAGE);
			}

			pass->Resolve();
		}

		std::for_each(resources_.begin(), resources_.end(), [](VirtualResource* resource) {
			if (resource->ref_count_)
			{
				PassNode* first = resource->first_;
				PassNode* last = resource->last_;
				if (first && last)
				{
					first->devirtualize_.insert(resource);
					last->destroy_.insert(resource);
				}
				if (resource->ref_count_)
				{
					Resource<RenderGraphTexture>* texture = static_cast<Resource<RenderGraphTexture>*>(resource);
					texture->resource_.desc_.usage |= rhi::TextureUsage::TRANSIENT_ATTACHMENT;
				}
			}
			});

		// Update final layout to Sampleable if necessary
		for (auto node : resource_nodes_)
		{
			PassNode* pass = node->GetWriterPass();
			if (pass && !node->IsCulled())
			{
				pass->UpdateAttachmentLayout(node->resource_index_);
			}
		}

		std::for_each(render_pass_path_.begin(), render_pass_path_.end(), [](PassNode* pass) {
			pass->Instantiate();
			});

		is_compiled_ = true;
	}

	void RenderGraph::Run(FrameResource& resource)
	{
		if (!is_compiled_)
			Compile();

		for (auto pass : render_pass_path_)
		{
			for (VirtualResource* virtual_resource : pass->devirtualize_)
			{
				virtual_resource->Instantiate();
			}

			pass->Execute(resource);

			for (VirtualResource* virtual_resource : pass->destroy_)
			{
				virtual_resource->Destroy(resource);
			}
		}
	}

	void RenderGraph::Read(PassNode* node, ResourceHandle handle)
	{
		for (auto it = resource_nodes_.rbegin(); it != resource_nodes_.rend(); ++it)
		{
			if ((*it)->resource_index_ == handle)
			{
				ResourceNode* resource_node = (*it);

				if (!node->is_subpass_)
				{
					RenderPassNode* rp_node = static_cast<RenderPassNode*>(node);
					ResourceNode* resource_node = new ResourceNode(*rp_node->subpass_graph_, handle);
					rp_node->subpass_graph_->resource_nodes_.push_back(resource_node);
				}

				DependencyGraph::Edge* edge = new DependencyGraph::Edge(graph_, (DependencyGraph::Node*)resource_node, (DependencyGraph::Node*)node);
				resource_node->SetOutgoingEdge(edge);

				/*VirtualResource* const resource = resources_[handle];
				resource->Connect(graph_, resource_node, node);*/
				break;
			}
		}
	}

	void RenderGraph::Write(PassNode* node, ResourceHandle handle)
	{
		ResourceNode* resource_node = new ResourceNode(*this, handle);

		resource_nodes_.push_back(resource_node);

		DependencyGraph::Edge* edge = new DependencyGraph::Edge(graph_, (DependencyGraph::Node*)node, (DependencyGraph::Node*)resource_node);
		resource_node->SetIncomingEdge(edge);

		/*VirtualResource* const resource = resources_[handle];

		resource->Connect(graph_, node, resource_node);*/
	}

	void RenderGraph::SetPipelineInternal(PassNode* node, rhi::RHIPipeline::Descriptor desc)
	{
		RenderPassNode* pass_node = static_cast<RenderPassNode*>(node);
		desc.render_pass = pass_node->pass_base_->actual_rp_;
		pass_node->pipelines_.push_back(desc);
	}

	void RenderGraph::SetPipelineInternal(SubpassNode* pass_node, const rhi::RHIPipeline::Descriptor& desc)
	{
		auto pipeline_desc = pass_node->parent_->pipelines_.emplace_back(desc);
		pipeline_desc.render_pass = pass_node->parent_->pass_base_->actual_rp_;
		pipeline_desc.subpass = pass_node->subpass_index_;
	}
}