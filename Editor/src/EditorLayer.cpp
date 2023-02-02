#include "mlepch.h"
#include "EditorLayer.h"
#include "Runtime/Core/Base/Layer.h"
#include "Runtime/Utils/BitmaskEnum.h"

#include <glm/gtc/type_ptr.hpp>

// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment)
{
	void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

namespace editor {
	void EditorLayer::OnAttach()
	{
		//////////////////////////////////////
		// Member Init
		////////////////////////////////////// 
		{
			desc_allocator_ = rhi::RHI::GetRHIInstance().CreateDescriptorAllocator();
			layout_cache_ = rhi::RHI::GetRHIInstance().CreateDescriptorSetLayoutCache();

			global_set_layout_ =
				rhi::DescriptorSetLayoutBuilder::Begin(layout_cache_.get())
				.AddBinding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.AddBinding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.Build();

			texture_set_layout_ =
				rhi::DescriptorSetLayoutBuilder::Begin(layout_cache_.get())
				.AddBinding(0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT_BIT, 1)
				.Build();

			for (auto i = 0; i < renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT; ++i)
			{
				rhi::RHIBuffer::Descriptor camera_ubo_desc{};
				camera_ubo_desc.element_stride = sizeof(CameraUbo);
				camera_ubo_desc.memory_usage = MemoryUsage::MEMORY_USAGE_CPU_TO_GPU;
				camera_ubo_desc.usage = ResourceTypes::RESOURCE_TYPE_UNIFORM_BUFFER;
				camera_ubo_desc.mapped_at_creation = true;
				camera_ubo_[i] = rhi::RHI::GetRHIInstance().RHICreateBuffer(camera_ubo_desc);

				rhi::RHIBuffer::Descriptor param_ubo_desc{};
				param_ubo_desc.element_stride = sizeof(AtmosphereParameter);
				param_ubo_desc.memory_usage = MemoryUsage::MEMORY_USAGE_CPU_TO_GPU;
				param_ubo_desc.usage = ResourceTypes::RESOURCE_TYPE_UNIFORM_BUFFER;
				param_ubo_desc.mapped_at_creation = true;
				param_ubo_[i] = rhi::RHI::GetRHIInstance().RHICreateBuffer(camera_ubo_desc);

				global_set_[i] = rhi::RHI::GetRHIInstance().RHICreateDescriptorSet();

				texture_set_[i] = rhi::RHI::GetRHIInstance().RHICreateDescriptorSet();;

				// Update Descriptor Set Only Once(We dont need to change it at runtime
				rhi::DescriptorWriter::Begin(desc_allocator_.get())
					.WriteBuffer(0, camera_ubo_[i].get(), DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.WriteBuffer(1, param_ubo_[i].get(), DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					.Build(global_set_[i].get(), global_set_layout_.get());

				desc_allocator_->Allocate(texture_set_[i].get(), texture_set_layout_.get());
			}
		}
		//////////////////////////////////////
		// Scene
		//////////////////////////////////////
		{
			editor_scene_ = std::make_shared<engine::Scene>();
			square_entity_ = editor_scene_->CreateEntity("square");
			square_entity_.AddComponent<engine::SpriteRendererComponent>();

			light_entity_ = editor_scene_->CreateEntity("light");
			light_entity_.AddComponent<engine::LightComponent>();

			scene_panel_.SetScene(editor_scene_);
		}

		//////////////////////////////////////
		// Renderer
		//////////////////////////////////////
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

			std::unique_ptr<rhi::ShaderModule> post_process_vert(rhi.RHICreateShaderModule("asset/shaders/PostProcess.spv"));
			std::unique_ptr<rhi::ShaderModule> atmosphere_frag(rhi.RHICreateShaderModule("asset/shaders/Atmosphere.spv"));
			std::unique_ptr<rhi::ShaderModule> combine_frag(rhi.RHICreateShaderModule("asset/shaders/Combine.spv"));

			rhi::DescriptorSetLayout* descriptor_sets_layout[] = { global_set_layout_.get()};

			rhi::PipelineLayout::Descriptor pipeline_layout_desc{};
			pipeline_layout_desc.set_layout_count = 1;
			pipeline_layout_desc.layouts = descriptor_sets_layout;
			atmosphere_pipeline_layout_ = rhi.RHICreatePipelineLayout(pipeline_layout_desc);

			rhi::RHIPipeline::Descriptor sky_pipeline{};
			sky_pipeline.vert_shader = post_process_vert.get();
			sky_pipeline.frag_shader = atmosphere_frag.get();
			sky_pipeline.layout = atmosphere_pipeline_layout_;
			sky_pipeline.use_vertex_attribute = false;

			rhi::DescriptorSetLayout* texture_descriptor_sets_layout[] = { texture_set_layout_.get() };

			rhi::PipelineLayout::Descriptor combine_pipeline_layout_desc{};
			combine_pipeline_layout_desc.set_layout_count = 1;
			combine_pipeline_layout_desc.layouts = texture_descriptor_sets_layout;
			combine_pipeline_layout_ = rhi.RHICreatePipelineLayout(combine_pipeline_layout_desc);

			rhi::RHIPipeline::Descriptor combine_pipeline{};
			combine_pipeline.vert_shader = post_process_vert.get();
			combine_pipeline.frag_shader = combine_frag.get();
			combine_pipeline.layout = combine_pipeline_layout_;
			combine_pipeline.use_vertex_attribute = false;

			renderer.LoadAllocator(desc_allocator_.get());
			renderer.LoadLayout(global_set_layout_.get());

			//--------------------------------------------------------------------------
			
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
								});
					},
					[=](RenderGraph& rg, rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
					{
						// Update Buffer
						CameraUbo camera_data{};
						;		camera_data.position = editor_camera_.GetPosition();
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

			rhi.RHIFreeShaderModule(*post_process_vert);
			rhi.RHIFreeShaderModule(*atmosphere_frag);
			rhi.RHIFreeShaderModule(*combine_frag);
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
		rhi.RHIFreePipelineLayout(*atmosphere_pipeline_layout_);
		delete atmosphere_pipeline_layout_;

		rhi.RHIFreePipelineLayout(*combine_pipeline_layout_);
		delete combine_pipeline_layout_;

		for (auto i = 0; i < renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT; ++i)
		{
			rhi.RHIFreeBuffer(*param_ubo_[i]);
			rhi.RHIFreeBuffer(*camera_ubo_[i]);
		}
		rhi.RHIFreeBuffer(*vb_);
		rhi.RHIFreeBuffer(*indicies_);

		rhi.RHIFreeTexture(*back_buffer_);
	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		using namespace renderer;
		Renderer& renderer = Renderer::GetInstance();
		frame_index_ = renderer.GetFrameIndex();
		auto& current_frame = renderer.GetCurrentFrame();
		// Resize
		if ( viewport_size_.x > 0.0f && viewport_size_.y > 0.0f && // zero sized framebuffer is invalid
			(back_buffer_->width != viewport_size_.x || back_buffer_->height != viewport_size_.y))
		{
			rhi::RHI::GetRHIInstance().ResizeTexture(*back_buffer_, (uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);

			RenderGraph& render_graph = renderer.GetRenderGraph();
			auto& pass = render_graph.GetRenderPass("Sky Pass");
			render_graph.ResizeRenderTarget(&pass, viewport_size_.x / 2, viewport_size_.y / 2);

			auto& combine_pass = render_graph.GetRenderPass("Combine Pass");
			render_graph.ResizeRenderTarget(&combine_pass, viewport_size_.x, viewport_size_.y);

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
			ImGui::DragFloat("Sea Level: ", &param_.sea_level);
			ImGui::DragFloat3("Planet Center: ", glm::value_ptr(param_.planet_center));
			ImGui::DragFloat("Planet Radius: ", &param_.planet_radius, 1000.0f, 0.0f);
			ImGui::DragFloat("Atmosphere height: ", &param_.atmosphere_height);
			ImGui::DragFloat("Sun Disk Angle: ", &param_.sun_disk_angle, 0.1f);
			ImGui::DragFloat("Rayleigh Scattering scale: ", &param_.rayleigh_scattering_scale);
			ImGui::DragFloat("Rayleigh Scattering scalar height: ", &param_.rayleigh_scattering_scalar_height);
			ImGui::DragFloat("Mie Scattering Scale: ", &param_.mie_scattering_scale);
			ImGui::DragFloat("Mie Anisotropy: ", &param_.mie_anisotropy, 0.01f, 0.0f, 1.0f);
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


		back_buffer_->RegisterForImGui();
		ImGui::Image((ImTextureID)back_buffer_->texture_id, ImVec2{ (float)back_buffer_->width, (float)back_buffer_->height}, ImVec2{0, 1}, ImVec2{1, 0});

		ImGui::End();
		ImGui::PopStyleVar();

		scene_panel_.OnUIRender();
	}
}