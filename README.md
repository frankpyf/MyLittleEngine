# MyLittleEngine
Pastiche, WIP(still in very early stage)  
Only support Windows for now, but cross-platform suppport will be added in the future.

### Render Graph
Keep in mind that it is in very early stage  
WIP
- [x] Basic Structure
- [ ] Render Pass Dependency

Example code  
```C++
// Delcare Resource
auto color_buffer = render_graph.RegisterTransientResource();
back_buffer_ = rhi.RHICreateTexture2D(1000,800,rhi::PixelFormat::RGBA);
auto back_buffer_handle = render_graph.RegisterResource(back_buffer_.get());


const std::vector<resource::Vertex> vertices = {
{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};
vb_ = renderer.LoadModel(vertices);

 // Triangle Pass
{
	RenderPassDesc tri_pass{};
	ColorAttachmentDesc& color = tri_pass.color_attachments.emplace_back();
	color.load_op = LoadOp::CLEAR;
	color.store_op = StoreOp::STORE;
	color.final_layout = ImageLayout::IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	color.format = AttachmentFormat::RGBA8;

	tri_pass.subpasses.push_back({ false, {0},{}});
	render_graph.AddRenderPass("Triangle Pass", tri_pass,
		[&](RenderPass* rp, RenderTargetDesc& rt)
		{
			rt.attachments_index.push_back(back_buffer_handle);
			rt.clear_value = { 0.0f,0.0f,0.0f,1.0f };

			PipelineDesc PSODesc{};
			PSODesc.render_pass = rp;
			PSODesc.subpass = 0;
			rp->CreatePipeline("asset/shaders/vert.spv", "asset/shaders/frag.spv", PSODesc);

			// Setup Dependencies between render passes
		},
		[&](RenderPass& rp, RenderTarget& rt, rhi::CommandBuffer& cmd_buffer)
		{
			BeginRenderPass(cmd_buffer, rp, rt);
			BindGfxPipeline(cmd_buffer, rp.GetPipeline(0));
			SetViewport(cmd_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
			SetScissor(cmd_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
			rhi::RHIVertexBuffer* vbs[] = { vb_.get() };
			uint64_t offsets[] = { 0 };

			BindVertexBuffers(cmd_buffer, 0, 1, vbs, offsets);
			Draw(cmd_buffer, 3, 1);
			EndRenderPass(cmd_buffer);
		});
}

// Present Pass
{
	RenderPassDesc present_pass{};
	present_pass.is_for_present = true;

	ColorAttachmentDesc& color = present_pass.color_attachments.emplace_back();
	color.load_op = LoadOp::CLEAR;
	color.store_op = StoreOp::STORE;
	color.final_layout = ImageLayout::IMAGE_LAYOUT_PRESENT;
	color.format = AttachmentFormat::SWAPCHAIN_FORMAT;
	present_pass.subpasses.push_back({ false, {0},{} });
	render_graph.AddRenderPass("Present & UI Pass", present_pass,
		[=](RenderPass* rp, RenderTargetDesc& rt)
		{
			rt.clear_value = { 0.0f,0.0f,0.0f,1.0f };
		},
		[](RenderPass& rp, RenderTarget& rt, rhi::CommandBuffer& cmd_buffer)
		{
			// Rendering
			ImGui::Render();
			ImDrawData* main_draw_data = ImGui::GetDrawData();
			const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
			if (!main_is_minimized)
			{
				BeginRenderPass(cmd_buffer, rp, rt);
				// Record dear imgui primitives into command buffer
				ImGui_RenderDrawData(cmd_buffer, main_draw_data);
				EndRenderPass(cmd_buffer);
			}
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		});
}
```
This is what it looks like(btw why imgui is kinda gray, still trying to figure it out): 
![Just a test](/Docs/Pictures/wip.PNG)

# Referencing
[Walnut(The Cherno)](https://github.com/TheCherno/Walnut)  
[Hazel(The Cherno)](https://github.com/TheCherno/Hazel)  
[Piccolo(Booming tech)](https://github.com/BoomingTech/Piccolo)   
[游戏引擎随笔 0x11：现代图形 API 特性的统一：Attachment Fetch](https://zhuanlan.zhihu.com/p/131392827)  
[游戏引擎随笔 0x03：可扩展的渲染管线架构](https://zhuanlan.zhihu.com/p/70668533)