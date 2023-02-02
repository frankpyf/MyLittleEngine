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
back_buffer_ = rhi.RHICreateTexture({ 800, 800, 1, 1, 1, PixelFormat::RGBA8, TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE});

ResourceHandle color_buffer_handle = render_graph.ImportResource<RenderGraphTexture>("back_buffer", 
	{ back_buffer_->width, back_buffer_->height, 1, 1, 1,  PixelFormat::RGBA8, TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE},
	{ back_buffer_ });
ResourceHandle sky_texture_handle = render_graph.AddResource<RenderGraphTexture>("sky_texture",
	{ (back_buffer_->width) / 2, (back_buffer_->height) / 2, 1, 1, 1,  PixelFormat::RGBA8, TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE });

	// Sky Pass
	{
		render_graph.AddPass("Sky Pass",
			[&](RenderGraph& rg, RenderGraph::RenderPassBuilder& builder)
			{
				builder.Write(sky_texture_handle, rhi::RenderPass::AttachmentDesc::LoadOp::CLEAR, rhi::RenderPass::AttachmentDesc::StoreOp::STORE)
					.AddSubpass("Atmosphere subpass",
						[&](RenderGraph& rg, RenderGraph::SubpassBuilder& builder)
						{
							builder.Write(sky_texture_handle)
								.SetPipeline(sky_pipeline);
							);
			},
			[=](RenderGraph& rg, rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
			{
				// Update Buffer
				CameraUbo camera_data{};
				camera_data.position = editor_camera_.GetPosition();
				camera_data.inverse_view = editor_camera_.GetInverseView();
				camera_data.inverse_proj = editor_camera_.GetInverseProjection();
				camera_ubo_[frame_index_]->SetData(&camera_data, sizeof(camera_data));

				param_.sun_light_color = light_entity_.GetComponent<engine::LightComponent>().light_color;
				param_.sun_light_intensity = light_entity_.GetComponent<engine::LightComponent>().light_intensity;

				glm::mat4 quat_rotation = glm::toMat4(glm::quat(light_entity_.GetComponent<engine::TransformComponent>().rotation));

				param_.sun_light_direction = quat_rotation * glm::vec4(0, 1, 1, 1);

				param_ubo_[frame_index_]->SetData(&param_, sizeof(param_));
				// ----------------------------
				BindGfxPipeline(*current_frame.command_buffer, rp.GetPipeline(0).get());
				SetViewport(*current_frame.command_buffer, 0, 0, (back_buffer_->width) / 2, (back_buffer_->height) / 2);
				SetScissor(*current_frame.command_buffer, 0, 0, (back_buffer_->width) / 2, (back_buffer_->height) / 2);

				rhi::DescriptorSet* sets[] = { global_set_[frame_index_].get()};
				BindDescriptorSets(*current_frame.command_buffer, rp.GetPipeline(0)->layout, 0, 1, sets, 0, nullptr);
				Draw(*current_frame.command_buffer, 3, 1, 0, 0);
			});
	}

	// Another Post Process Pass
	{
		render_graph.AddPass("Combine Pass",
			[&](RenderGraph& rg, RenderGraph::RenderPassBuilder& builder)
			{
				builder.Read(sky_texture_handle)
					.Write(color_buffer_handle, rhi::RenderPass::AttachmentDesc::LoadOp::DONT_CARE, rhi::RenderPass::AttachmentDesc::StoreOp::STORE)
					.AddSubpass("Combine subpass",
						[&](RenderGraph& rg, RenderGraph::SubpassBuilder& builder)
						{
							builder.Read(0, 0, sky_texture_handle)
								.Write(color_buffer_handle)
								.SetPipeline(combine_pipeline);
						});
			},
			[=](RenderGraph& rg, rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
			{
				auto sky_texture = rg.GetResource(sky_texture_handle);
				auto sky_texture_resource = static_cast<Resource<RenderGraphTexture>*>(sky_texture);

				rhi::DescriptorWriter::Begin(desc_allocator_.get())
					.WriteImage(0, sky_texture_resource->resource_.texture.get(), DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
					.OverWrite(texture_set_[frame_index_].get());
				// ----------------------------
				BindGfxPipeline(*current_frame.command_buffer, rp.GetPipeline(0).get());
				SetViewport(*current_frame.command_buffer, 0, 0, back_buffer_->width, back_buffer_->height);
				SetScissor(*current_frame.command_buffer, 0, 0, back_buffer_->width, back_buffer_->height);

				rhi::DescriptorSet* sets[] = { texture_set_[frame_index_].get()};
				BindDescriptorSets(*current_frame.command_buffer, rp.GetPipeline(0)->layout, 0, 1, sets, 0, nullptr);
				Draw(*current_frame.command_buffer, 3, 1, 0, 0);
			});
	}
		
	// Present Pass
	{
		render_graph.AddPresentPass("Present & UI Pass",
			[&](RenderGraph& rg, RenderGraph::RenderPassBuilder& builder)
			{
				builder.Read(color_buffer_handle);
			},
			[=](RenderGraph& rg, rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
			{
				// Rendering
				ImGui::Render();
				ImDrawData* main_draw_data = ImGui::GetDrawData();
				const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
						
				// Record dear imgui primitives into command buffer
				ImGui_RenderDrawData(*current_frame.command_buffer, main_draw_data);
						
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}
			});
	}

			
render_graph.Compile();
```
This is what it looks like(btw why imgui is kinda gray, still trying to figure it out): 
![Just a test](/Docs/Pictures/PBR_Atmosphere.PNG)

# Referencing
[Walnut(The Cherno)](https://github.com/TheCherno/Walnut)  
[Hazel(The Cherno)](https://github.com/TheCherno/Hazel)  
[Piccolo(Booming tech)](https://github.com/BoomingTech/Piccolo)   
[游戏引擎随笔 0x11：现代图形 API 特性的统一：Attachment Fetch](https://zhuanlan.zhihu.com/p/131392827)  
[游戏引擎随笔 0x03：可扩展的渲染管线架构](https://zhuanlan.zhihu.com/p/70668533)