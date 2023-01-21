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

	}
	void EditorLayer::OnAttach()
	{
		//////////////////////////////////////
		// Scene
		//////////////////////////////////////
		{
			editor_scene_ = std::make_shared<engine::Scene>();
			square_entity_ = editor_scene_->CreateEntity("square");
			square_entity_.AddComponent<engine::TransformComponent>();
			square_entity_.AddComponent<engine::SpriteRendererComponent>();
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

			// desc_allocator_ = rhi.CreateDescriptorAllocator();
			//layout_cache_ = rhi.CreateDescriptorSetLayoutCache();

			global_set_layout_ = 
				rhi::DescriptorSetLayoutBuilder::Begin(layout_cache_.get())
				.AddBinding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX_BIT, 1)
				.Build();

			vert_shader_ = rhi.RHICreateShaderModule("asset/shaders/vert.spv");
			frag_shader_ = rhi.RHICreateShaderModule("asset/shaders/frag.spv");

			rhi::DescriptorSetLayout* descriptor_sets_layout[1] = { global_set_layout_ };

			rhi::PipelineLayout::Descriptor pipeline_layout_desc{};
			pipeline_layout_desc.set_layout_count = 1;
			pipeline_layout_desc.layouts = descriptor_sets_layout;
			pipeline_layout_ = rhi.RHICreatePipelineLayout(pipeline_layout_desc);

			rhi::RHIPipeline::Descriptor tri_pipeline{};
			tri_pipeline.vert_shader = vert_shader_;
			tri_pipeline.frag_shader = frag_shader_;
			tri_pipeline.layout = pipeline_layout_;

			vb_ = renderer.LoadModel(vertices);
			const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
			indicies_ = renderer.LoadIndex(indices);

			renderer.LoadAllocator(desc_allocator_.get());
			renderer.LoadLayout(global_set_layout_);


			//--------------------------------------------------------------------------
			
			// Delcare Resource
			back_buffer_ = rhi.RHICreateTexture2D({ 800, 800, 1, rhi::PixelFormat::RGBA8, rhi::TextureUsage::COLOR_ATTACHMENT | rhi::TextureUsage::SAMPLEABLE});

			ResourceHandle color_buffer_handle = render_graph.ImportResource<RenderGraphTexture>("back_buffer", 
				{ 800, 800, 1, rhi::PixelFormat::RGBA8, rhi::TextureUsage::COLOR_ATTACHMENT | rhi::TextureUsage::SAMPLEABLE },
				{ back_buffer_ });

			ResourceHandle depth_buffer_handle = render_graph.AddResource<RenderGraphTexture>("depth_buffer", { 800, 800, 1, rhi::PixelFormat::DEPTH });

			 // Triangle Pass
			{
				render_graph.AddPass("Triangle Pass",
					[&](RenderGraph& rg, RenderGraph::Builder& builder)
					{
						builder.Write(color_buffer_handle, rhi::RenderPass::AttachmentDesc::LoadOp::CLEAR, rhi::RenderPass::AttachmentDesc::StoreOp::STORE)
							.AddSubpass("Triangle subpass",
							[&](RenderGraph& rg, RenderGraph::SubpassBuilder& builder)
							{
								builder.Write(color_buffer_handle);
							})
							.SetPipeline(tri_pipeline);

					},
					[=](rhi::RenderPass& rp, rhi::RenderTarget& rt, FrameResource& current_frame)
					{
						BindGfxPipeline(*current_frame.command_buffer, rp.GetPipeline(0).get());
						SetViewport(*current_frame.command_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
						SetScissor(*current_frame.command_buffer, 0, 0, back_buffer_->GetWidth(), back_buffer_->GetHeight());
						{
							resource::UniformBufferObject ubo{};
							
							ubo.view = editor_camera_.GetView();
							ubo.proj = editor_camera_.GetProjection();

							auto view = editor_scene_->GetAllEntitiesWith<engine::TransformComponent, engine::SpriteRendererComponent>();
							
							for (auto entity : view)
							{
								auto [transform, sprite] = view.get<engine::TransformComponent, engine::SpriteRendererComponent>(entity);

								ubo.model = transform.GetTransform();
								current_frame.global_ubo->SetData(&ubo, sizeof(ubo));

								rhi::RHIBuffer* vbs[] = { vb_.get() };
								uint64_t offsets[] = { 0 };
								BindVertexBuffers(*current_frame.command_buffer, 0, 1, vbs, offsets);
								BindIndexBuffer(*current_frame.command_buffer, indicies_.get(), 0);

								rhi::DescriptorSet* sets[1] = { current_frame.global_set };

								BindDescriptorSets(*current_frame.command_buffer, rp.GetPipeline(0)->layout, 0, 1, sets, 0, nullptr);

								DrawIndexed(*current_frame.command_buffer, 6, 1, 0, 0, 0);
							}
						}
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
			
			render_graph.Compile();
		}
	}

	void EditorLayer::OnDetach() 
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();

		// Temp
		rhi.RHIFreePipelineLaoyout(pipeline_layout_);
		delete pipeline_layout_;
		rhi.RHIFreeShaderModule(vert_shader_);
		rhi.RHIFreeShaderModule(frag_shader_);
		delete vert_shader_;
		delete frag_shader_;

		// ---------------------------------
		using namespace renderer;
		Renderer& renderer = Renderer::GetInstance();

		RenderGraph& render_graph = renderer.GetRenderGraph();

		render_graph.Clear();

	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		using namespace renderer;
		Renderer& renderer = Renderer::GetInstance();
		auto& current_frame = renderer.GetCurrentFrame();
		// Resize
		if ( viewport_size_.x > 0.0f && viewport_size_.y > 0.0f && // zero sized framebuffer is invalid
			(back_buffer_->GetWidth() != viewport_size_.x || back_buffer_->GetHeight() != viewport_size_.y))
		{
			back_buffer_->Resize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);

			RenderGraph& render_graph = renderer.GetRenderGraph();
			auto& pass = render_graph.GetRenderPass("Triangle Pass");
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

		ImGui::Begin("Panel 1");
		ImGui::Button("Button");
		ImGui::Separator();
		// ---------------
		ImGui::DragFloat3("Translation", glm::value_ptr(square_entity_.GetComponent<engine::TransformComponent>().translation));
		ImGui::DragFloat3("Rotation", glm::value_ptr(square_entity_.GetComponent<engine::TransformComponent>().rotation));
		ImGui::DragFloat3("Scale", glm::value_ptr(square_entity_.GetComponent<engine::TransformComponent>().scale));
		// ---------------
		ImGui::End();

		ImGui::Begin("Panel 2");
		
		ImGui::End();
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		viewport_size_ = { viewportPanelSize.x, viewportPanelSize.y };

		ImGui::Image((ImTextureID)back_buffer_->GetTextureID(), ImVec2{ (float)back_buffer_->GetWidth(), (float)back_buffer_->GetHeight()}, ImVec2{0, 1}, ImVec2{1, 0});

		ImGui::End();
		ImGui::PopStyleVar();
	}
}