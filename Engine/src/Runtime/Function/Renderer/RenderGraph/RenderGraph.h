#pragma once
#include "Runtime/Core/Base/Singleton.h"
#include "DependencyGraph.h"
#include "Nodes.h"
#include "RenderGraphPass.h"
#include "VirtualResource.h"

namespace renderer{
	class Renderer;
	using ResourceHandle = size_t;

	class RenderGraph
	{
		friend class RenderGraphEditor;
		friend class Renderer;
	public:
		class SubpassBuilder
		{
			friend class Builder;
		public:
			SubpassBuilder(RenderGraph& rg, SubpassNode* node)
				:rg_(rg), subpass_node_(node) {};
			SubpassBuilder(SubpassBuilder const&) = delete;
			SubpassBuilder& operator=(SubpassBuilder const&) = delete;

			SubpassBuilder& Read(uint32_t set, uint32_t binding, ResourceHandle resource);
			SubpassBuilder& Write(ResourceHandle resource);
			SubpassBuilder& SetPipeline(const rhi::RHIPipeline::Descriptor& desc);
		private:
			//subpass_graph
			RenderGraph& rg_;
			SubpassNode* const subpass_node_;
		};
		class RenderPassBuilder
		{
			friend class RenderGraph;
			using LoadOp = rhi::RenderPass::AttachmentDesc::LoadOp;
			using StoreOp = rhi::RenderPass::AttachmentDesc::StoreOp;

		public:
			RenderPassBuilder(RenderGraph& rg, RenderPassNode* node)
				:rg_(rg), node_(node) {};
			RenderPassBuilder(RenderPassBuilder const&) = delete;
			RenderPassBuilder& operator=(RenderPassBuilder const&) = delete;

			RenderPassBuilder& Read(uint32_t set, uint32_t binding, ResourceHandle resource);
			RenderPassBuilder& Read(ResourceHandle resource);
			RenderPassBuilder& ReadWrite(uint32_t set, uint32_t binding, ResourceHandle resource, LoadOp load_operation, StoreOp store_operation);
			RenderPassBuilder& Write(ResourceHandle resource, LoadOp load_operation, StoreOp store_operation);

			RenderPassBuilder& SetPipeline(const rhi::RHIPipeline::Descriptor& desc);

			template<typename Setup>
			RenderPassBuilder& AddSubpass(const char* pass_name, Setup setup)
			{
				RenderPassNode* rp_node = static_cast<RenderPassNode*>(node_);
				auto graph = rp_node->subpass_graph_;
				SubpassBuilder builder(graph->AddSubPassInternal(pass_name, rp_node));
				setup(*graph, builder);

				return *this;
			};
		private:
			RenderGraph& rg_;
			RenderPassNode* node_;
		};
		// --------------------------------------------------

		virtual ~RenderGraph() = default;

		void SetRenderer(Renderer* in_renderer);

		PassNode& GetRenderPass(const char* render_pass_name);

		/// <summary>
		/// Add a render pass to the render graph, wip: compute pass
		/// </summary>
		/// <typeparam name="Setup">Setup lambda</typeparam>
		/// <typeparam name="Execute">Execution lambda</typeparam>
		/// <param name="pass_name">name of this pass</param>
		/// <param name="setup">setup function, using builder</param>
		/// <param name="execute">execution function, using render commands</param>
		template<typename Setup, typename Execute>
		void AddPass(const char* pass_name, Setup setup, Execute execute)
		{
			auto* const pass = RenderGraphPassBase::Create<Execute>(execute);

			RenderPassBuilder builder(AddRenderPassInternal(pass_name, pass));
			setup(*this, builder);

			is_compiled_ = false;
		}

		template<typename Setup, typename Execute>
		void AddPresentPass(const char* pass_name, Setup setup, Execute execute)
		{

			auto* const pass = RenderGraphPassBase::Create<Execute>(execute);

			RenderPassNode* node = new PresentPassNode(pass_name, *this, pass);
			pass->SetNode(node);
			pass_nodes_.push_back(node);
			node->DontCull();

			RenderPassBuilder builder(*this, node);
			setup(*this, builder);

			is_compiled_ = false;
		}

		RenderPassBuilder AddRenderPassInternal(const char* name, RenderGraphPassBase* base);
		SubpassBuilder AddSubPassInternal(const char* name, RenderPassNode* parent);

		void RemoveRenderPass(const char* render_pass_name);

		void Clear();

		void Run(FrameResource& resource);

		void Compile();

		template<typename RESOURCE>
		ResourceHandle ImportResource(const char* name, 
			typename RESOURCE::Descriptor const& desc,						  
			typename RESOURCE const& resource)
		{
			Resource<RESOURCE>* imported_resource = new Resource<RESOURCE>(name, desc, resource);
			resources_.push_back(imported_resource);
			return resources_.size() - 1;
		}

		template<typename RESOURCE>
		ResourceHandle AddResource(const char* name, typename RESOURCE::Descriptor const& descriptor)
		{
			Resource<RESOURCE>* virtual_resource = new Resource<RESOURCE>(name, descriptor);
			resources_.push_back(virtual_resource);
			return resources_.size() - 1;
		}

		void Read(RenderPassNode* pass_node, ResourceHandle handle);
		void Read(uint32_t set, uint32_t binding, PassNode* pass_node, ResourceHandle handle);
		void Write(PassNode* pass_node, ResourceHandle handle);
		void SetPipelineInternal(PassNode* node, rhi::RHIPipeline::Descriptor desc);
		void SetPipelineInternal(SubpassNode* pass_node, const rhi::RHIPipeline::Descriptor& desc);

		VirtualResource* GetResource(ResourceHandle handle)
		{
			return resources_[handle];
		}

		// TEMP
		void ResizeRenderTarget(PassNode* node, uint32_t width, uint32_t height)
		{
			RenderPassNode* pass = static_cast<RenderPassNode*>(node);
			pass->render_target_.desc_.width = width;
			pass->render_target_.desc_.height = height;
			std::for_each(pass->render_target_.desc_.attachments.begin(), pass->render_target_.desc_.attachments.end(), [=](RenderGraphTexture* attachment) {
				attachment->desc_.width = width;
				attachment->desc_.height = height;
				});
		}

		inline DependencyGraph& GetGraph() { return graph_; };

		Renderer* renderer_ = nullptr;
	private:
		std::vector<PassNode*> pass_nodes_{};
		std::vector<PassNode*> render_pass_path_{};
		std::vector<ResourceNode*> resource_nodes_{};

		std::vector<VirtualResource*> resources_{};

		DependencyGraph graph_;

		bool is_compiled_ = false;
	};

}