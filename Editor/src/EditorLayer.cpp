#include "mlepch.h"
#include "EditorLayer.h"
#include "Runtime/Core/Base/Layer.h"
#include "Runtime/Utils/BitmaskEnum.h"

#include <glm/gtc/type_ptr.hpp>

namespace editor {
	EditorLayer::EditorLayer()
		:desc_allocator_(rhi::RHI::GetRHIInstance().CreateDescriptorAllocator()),
		layout_cache_(rhi::RHI::GetRHIInstance().CreateDescriptorSetLayoutCache())
	{
		for (uint8_t i = 0; i < renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			/*texture_set_[i] = rhi::RHI::GetRHIInstance().CreateDescriptorSet();
			buffer_set_[i] = rhi::RHI::GetRHIInstance().CreateDescriptorSet();*/

			// Camera Ubo
			rhi::RHIBuffer::Descriptor camera_buffer_desc{};
			camera_buffer_desc.size = sizeof(CameraUbo);
			camera_buffer_desc.mapped_at_creation = true;
			camera_buffer_desc.memory_usage = MemoryUsage::MEMORY_USAGE_CPU_TO_GPU;
			camera_buffer_desc.usage = ResourceTypes::RESOURCE_TYPE_UNIFORM_BUFFER;
			camera_ubo_[i] = rhi::RHI::GetRHIInstance().RHICreateBuffer(camera_buffer_desc);

			// Param Ubo
			rhi::RHIBuffer::Descriptor buffer_desc{};
			buffer_desc.size = sizeof(AtmosphereParameter);
			buffer_desc.mapped_at_creation = true;
			buffer_desc.memory_usage = MemoryUsage::MEMORY_USAGE_CPU_TO_GPU;
			buffer_desc.usage = ResourceTypes::RESOURCE_TYPE_UNIFORM_BUFFER;
			param_ubo_[i] = rhi::RHI::GetRHIInstance().RHICreateBuffer(buffer_desc);

			global_set_[i] = rhi::RHI::GetRHIInstance().CreateDescriptorSet();
		}
	}
	void EditorLayer::OnAttach()
	{
		//////////////////////////////////////
		// Scene
		//////////////////////////////////////
		{
			editor_scene_ = std::make_shared<engine::Scene>();
			square_entity_ = editor_scene_->CreateEntity("square");
			square_entity_.AddComponent<engine::SpriteRendererComponent>();

			light_entity_ = editor_scene_->CreateEntity("light");

			scene_panel_.SetScene(editor_scene_);
		}

		{
			using namespace renderer;

			rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
			Renderer& renderer = Renderer::GetInstance();

			RenderGraph& render_graph = renderer.GetRenderGraph();

			// temp
			const std::vector<resource::Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
			};
			vb_ = renderer.LoadModel(vertices);
			const std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };
			indicies_ = renderer.LoadIndex(indices);

			
			/*texture_set_layout_ =
				rhi::DescriptorSetLayoutBuilder::Begin(layout_cache_.get())
				.AddBinding(0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.AddBinding(1, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.AddBinding(2, DESCRIPTOR_TYPE_INPUT_ATTACHMENT, SHADER_STAGE_FRAGMENT_BIT, 1)
				.Build();
			buffer_set_layout_ =
				rhi::DescriptorSetLayoutBuilder::Begin(layout_cache_.get())
				.AddBinding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.Build();*/

			//std::unique_ptr<rhi::ShaderModule> vert_shader(rhi.RHICreateShaderModule("asset/shaders/vert.spv"));
			//std::unique_ptr<rhi::ShaderModule> frag_shader(rhi.RHICreateShaderModule("asset/shaders/frag.spv"));

			std::unique_ptr<rhi::ShaderModule> post_process_vert(rhi.RHICreateShaderModule("asset/shaders/PostProcess.spv"));
			std::unique_ptr<rhi::ShaderModule> atmosphere_frag(rhi.RHICreateShaderModule("asset/shaders/Atmosphere.spv"));
			//std::unique_ptr<rhi::ShaderModule> combine_frag(rhi.RHICreateShaderModule("asset/shaders/Combine.spv"));

			global_set_layout_ =
				rhi::DescriptorSetLayoutBuilder::Begin(layout_cache_.get())
				.AddBinding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.AddBinding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.Build();

			rhi::DescriptorSetLayout* descriptor_sets_layout[] = { global_set_layout_ };

			rhi::PipelineLayout::Descriptor pipeline_layout_desc{};
			pipeline_layout_desc.set_layout_count = 1;
			pipeline_layout_desc.layouts = descriptor_sets_layout;
			atmosphere_pipeline_layout_ = rhi.RHICreatePipelineLayout(pipeline_layout_desc);

			/*rhi::DescriptorSetLayout* descriptor_sets_layouts[] = { global_set_layout_, texture_set_layout_, buffer_set_layout_ };

			rhi::PipelineLayout::Descriptor pipeline_layout_desc{};
			pipeline_layout_desc.set_layout_count = 3;
			pipeline_layout_desc.layouts = descriptor_sets_layouts;
			atmosphere_pipeline_layout_ = rhi.RHICreatePipelineLayout(pipeline_layout_desc);

			rhi::DescriptorSetLayout* descriptor_sets_layouts[] = { texture_set_layout_};

			rhi::PipelineLayout::Descriptor pipeline_layout_desc{};
			pipeline_layout_desc.set_layout_count = 1;
			pipeline_layout_desc.layouts = descriptor_sets_layouts;
			combine_pipeline_layout_ = rhi.RHICreatePipelineLayout(pipeline_layout_desc);*/

			/*rhi::RHIPipeline::Descriptor tri_pipeline{};
			tri_pipeline.vert_shader = vert_shader.get();
			tri_pipeline.frag_shader = frag_shader.get();
			tri_pipeline.layout = pipeline_layout_;*/

			rhi::RHIPipeline::Descriptor sky_pipeline{};
			sky_pipeline.vert_shader = post_process_vert.get();
			sky_pipeline.frag_shader = atmosphere_frag.get();
			sky_pipeline.layout = atmosphere_pipeline_layout_;
			sky_pipeline.use_vertex_attribute = false;

			/*rhi::RHIPipeline::Descriptor combine_pipeline{};
			sky_pipeline.vert_shader = post_process_vert.get();
			sky_pipeline.frag_shader = combine_frag.get();
			sky_pipeline.layout = combine_pipeline_layout_;
			sky_pipeline.use_vertex_attribute = false;*/

			renderer.LoadAllocator(desc_allocator_.get());
			renderer.LoadLayout(global_set_layout_);

			//--------------------------------------------------------------------------
			
			// Delcare Resource
			back_buffer_ = rhi.RHICreateTexture2D({ 800, 800, 1, rhi::PixelFormat::RGBA8, rhi::TextureUsage::COLOR_ATTACHMENT | rhi::TextureUsage::SAMPLEABLE});

			ResourceHandle color_buffer_handle = render_graph.ImportResource<RenderGraphTexture>("back_buffer", 
				{ back_buffer_->GetWidth(), back_buffer_->GetHeight(), 1, rhi::PixelFormat::RGBA8, rhi::TextureUsage::COLOR_ATTACHMENT | rhi::TextureUsage::SAMPLEABLE},
				{ back_buffer_ });

			// // Triangle Pass
			//{
			//	render_graph.AddPass("Triangle Pass",
			//		[&](RenderGraph& rg, RenderGraph::Builder& builder)
			//		{
			//			builder.Write(triangle_texture_handle, rhi::RenderPass::AttachmentDesc::LoadOp::CLEAR, rhi::RenderPass::AttachmentDesc::StoreOp::STORE)
			//				.Write(depth_buffer_handle, rhi::RenderPass::AttachmentDesc::LoadOp::CLEAR, rhi::RenderPass::AttachmentDesc::StoreOp::STORE)
			//				.AddSubpass("Triangle subpass",
			//				[&](RenderGraph& rg, RenderGraph::SubpassBuilder& builder)
			//				{
			//					builder.Write(triangle_texture_handle)
			//						.Write(depth_buffer_handle)
			//						.SetPipeline(tri_pipeline);
			//				});

			//		},
			//		[=](rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
			//		{
			//			BindGfxPipeline(*current_frame.command_buffer, rp.GetPipeline(0).get());
			//			SetViewport(*current_frame.command_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
			//			SetScissor(*current_frame.command_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
			//			{
			//				resource::UniformBufferObject ubo{};
			//				
			//				ubo.view = editor_camera_.GetView();
			//				ubo.proj = editor_camera_.GetProjection();

			//				auto view = editor_scene_->GetAllEntitiesWith<engine::TransformComponent, engine::SpriteRendererComponent>();
			//				
			//				for (auto entity : view)
			//				{
			//					auto [transform, sprite] = view.get<engine::TransformComponent, engine::SpriteRendererComponent>(entity);

			//					ubo.model = transform.GetTransform();
			//					current_frame.global_ubo->SetData(&ubo, sizeof(ubo));

			//					rhi::RHIBuffer* vbs[] = { vb_.get() };
			//					uint64_t offsets[] = { 0 };
			//					BindVertexBuffers(*current_frame.command_buffer, 0, 1, vbs, offsets);
			//					BindIndexBuffer(*current_frame.command_buffer, indicies_.get(), 0);

			//					rhi::DescriptorSet* sets[1] = { current_frame.global_set };

			//					BindDescriptorSets(*current_frame.command_buffer, rp.GetPipeline(0)->layout, 0, 1, sets, 0, nullptr);

			//					DrawIndexed(*current_frame.command_buffer, indices.size(), 1, 0, 0, 0);
			//				}
			//			}
			//		});
			//}

			// Sky Pass
			{
				render_graph.AddPass("Sky Pass",
					[&](RenderGraph& rg, RenderGraph::Builder& builder)
					{
						builder.Write(color_buffer_handle, rhi::RenderPass::AttachmentDesc::LoadOp::CLEAR, rhi::RenderPass::AttachmentDesc::StoreOp::STORE)
							.AddSubpass("Atmosphere subpass",
								[&](RenderGraph& rg, RenderGraph::SubpassBuilder& builder)
								{
									builder.Write(color_buffer_handle)
										.SetPipeline(sky_pipeline);
								});
					},
					[=](rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
					{
						// ----------------------------
						// Update Buffer
						CameraUbo camera{};
						camera.position = editor_camera_.GetPosition();
						camera.inverse_view = editor_camera_.GetInverseView();
						camera.inverse_proj = editor_camera_.GetInverseProjection();
						camera_ubo_[frame_index_]->SetData(&camera, sizeof(camera));

						param_ubo_[frame_index_]->SetData(&param_, sizeof(param_));
						// ----------------------------
						BindGfxPipeline(*current_frame.command_buffer, rp.GetPipeline(0).get());
						SetViewport(*current_frame.command_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
						SetScissor(*current_frame.command_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());

						rhi::DescriptorSet* sets[] = { global_set_[frame_index_] };
						BindDescriptorSets(*current_frame.command_buffer, rp.GetPipeline(0)->layout, 0, 1, sets, 0, nullptr);
						Draw(*current_frame.command_buffer, 3, 1, 0, 0);
					});
			}
		
			// Present Pass
			{
				render_graph.AddPresentPass("Present & UI Pass",
					[&](RenderGraph& rg, RenderGraph::Builder& builder)
					{
						builder.Read(color_buffer_handle);
					},
					[=](rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
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

			for (uint8_t i = 0; i < renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT; ++i)
			{
				// Update Descriptor Set
				rhi::DescriptorWriter::Begin(desc_allocator_.get())
					.WriteBuffer(0, camera_ubo_[i].get(), DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.WriteBuffer(1, param_ubo_[i].get(), DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.Build(global_set_[i], global_set_layout_);
			}
			
			render_graph.Compile();

			rhi.RHIFreeShaderModule(post_process_vert.get());
			rhi.RHIFreeShaderModule(atmosphere_frag.get());
		}


	}

	void EditorLayer::OnDetach() 
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		// ---------------------------------
		using namespace renderer;
		Renderer& renderer = Renderer::GetInstance();

		RenderGraph& render_graph = renderer.GetRenderGraph();

		render_graph.Clear();

		// ---------------------------------
		// Temp
		rhi.RHIFreePipelineLaoyout(atmosphere_pipeline_layout_);
		delete atmosphere_pipeline_layout_;


		for (uint8_t i = 0; i < renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			rhi.RHIFreeBuffer(param_ubo_[i]);
			rhi.RHIFreeBuffer(camera_ubo_[i]);
		}
		rhi.RHIFreeBuffer(vb_);
		rhi.RHIFreeBuffer(indicies_);
	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		using namespace renderer;
		Renderer& renderer = Renderer::GetInstance();
		frame_index_ = renderer.GetFrameIndex();
		auto& current_frame = renderer.GetCurrentFrame();
		// Resize
		if ( viewport_size_.x > 0.0f && viewport_size_.y > 0.0f && // zero sized framebuffer is invalid
			(back_buffer_->GetWidth() != viewport_size_.x || back_buffer_->GetHeight() != viewport_size_.y))
		{
			back_buffer_->Resize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);

			RenderGraph& render_graph = renderer.GetRenderGraph();
			auto& pass = render_graph.GetRenderPass("Sky Pass");
			render_graph.ResizeRenderTarget(&pass, viewport_size_.x, viewport_size_.y);

			editor_camera_.OnResize(viewport_size_.x, viewport_size_.y);
		}

		editor_camera_.OnUpdate(delta_time);
	}

	void EditorLayer::OnUIRender()
	{
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
				// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", nullptr, window_flags);
		ImGui::PopStyleVar();

		ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{ 
					/*NewScene();*/
				}
				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					//OpenScene();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					//SaveScene();
				}
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					//SaveSceneAs();
				}
				if (ImGui::MenuItem("Exit"))
				{
					engine::Application::GetApp().Close();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		ImGui::End();

		{
			ImGui::Begin("Panel 2");

			ImGui::End();
		}
		
		{
			ImGui::Begin("Atmosphere Properties");
			ImGui::DragFloat3("Sun Light Direction: ", glm::value_ptr(param_.sun_light_direction), 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("Sun Light Intensity: ", &param_.sun_light_intensity);
			ImGui::ColorEdit3("Sun Light Color: ", glm::value_ptr(param_.sun_light_color));
			ImGui::DragFloat("Sea Level: ", &param_.sea_level);
			ImGui::DragFloat3("Planet Center: ", glm::value_ptr(param_.planet_center));
			ImGui::DragFloat("Planet Radius: ", &param_.planet_radius, 1000.0f, 0.0f);
			ImGui::DragFloat("Atmosphere height: ", &param_.atmosphere_height);
			ImGui::DragFloat("Sun Disk Angle: ", &param_.sun_disk_angle, 0.1f);
			ImGui::DragFloat("Rayleigh Scattering scale: ", &param_.rayleigh_scattering_scale);
			ImGui::DragFloat("Rayleigh Scattering scalar height: ", &param_.rayleigh_scattering_scalar_height);
			ImGui::DragFloat("Mie Scattering Scale: ", &param_.mie_scattering_scale);
			ImGui::DragFloat("Mie Anisotropy: ", &param_.mie_anisotropy, 1.0f, 0.0f, 1.0f);
			ImGui::DragFloat("Mie Scattering Scalr Height: ", &param_.mie_scattering_scalar_height);
			ImGui::DragFloat("Ozone Level Center Height: ", &param_.ozone_level_center_height, 1.0f);
			ImGui::DragFloat("Ozone Level Width: ", &param_.ozone_level_width, 1.0f);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		viewport_size_ = { viewportPanelSize.x, viewportPanelSize.y };

		ImGui::Image((ImTextureID)back_buffer_->GetTextureID(), ImVec2{ (float)back_buffer_->GetWidth(), (float)back_buffer_->GetHeight()}, ImVec2{0, 1}, ImVec2{1, 0});

		ImGui::End();
		ImGui::PopStyleVar();

		scene_panel_.OnUIRender();
	}
}