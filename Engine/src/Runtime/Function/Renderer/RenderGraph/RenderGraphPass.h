#pragma once
#include "Runtime/Function/RHI/RenderPass.h"
#include "Runtime/Function/RHI/CommandBuffer.h"
#include "Runtime/Function/Renderer/RenderCommands.h"
#include "Runtime/Function/Renderer/FrameResource.h"

namespace renderer {
	class PassNode;

	class RenderGraphPassBase
	{
	public:
		RenderGraphPassBase() = default;
		~RenderGraphPassBase() = default;
		template<typename Execute>
		static RenderGraphPassBase* Create(const Execute& exec)
		{
			RenderGraphPass<Execute>* pass = new RenderGraphPass<Execute>(exec);
			return pass;
		};

		static RenderGraphPassBase* Create()
		{
			return new RenderGraphPassBase();
		}

		virtual void Instantiate()
		{
			actual_rp_ = rhi::RenderPass::Create(desc_);
		}

		virtual void Exec(rhi::RenderTarget& rt, FrameResource& frame) {};
		void SetNode(PassNode* node) { node_ = node; };

		using Descriptor = rhi::RenderPass::Descriptor;
		Descriptor desc_;

		rhi::RenderPass* actual_rp_ = nullptr;
	protected:
		PassNode* node_ = nullptr;
	};

	template<typename Execute>
	class RenderGraphPass : public RenderGraphPassBase
	{
	public:
		RenderGraphPass(const Execute& exec)
			:exec_func_(exec)
		{
		};

		virtual void Exec(rhi::RenderTarget& rt, FrameResource& frame) override
		{
			assert(actual_rp_!=nullptr&&"render pass is a null");
			
			BeginRenderPass(*frame.command_buffer, *actual_rp_, rt);

			exec_func_(*actual_rp_, rt, frame);

			EndRenderPass(*frame.command_buffer);
		}
	private:
		Execute exec_func_;
	};
}

