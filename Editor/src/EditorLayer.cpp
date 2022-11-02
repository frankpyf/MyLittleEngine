#include "mlepch.h"
#include "EditorLayer.h"
#include "Runtime/Core/Base/Layer.h"

#include "backends/imgui_impl_vulkan.h"

namespace editor {
	void EditorLayer::OnAttach()
	{
		{
			using namespace renderer;

			RenderGraph& render_graph = RenderGraph::GetInstance();
			rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
			// Delcare Resource
			auto color_buffer = render_graph.RegisterTransientResource();
			 back_buffer_ = rhi.RHICreateTexture2D(1000,800,rhi::PixelFormat::RGBA);
			 auto back_buffer_handle = render_graph.RegisterResource(back_buffer_.get());

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
						rt.width = (*back_buffer_).GetWidth();
						rt.height = (*back_buffer_).GetHeight();
						rt.clear_value = { 0.0f,0.0f,0.0f,1.0f };

						PipelineDesc PSODesc{};
						PSODesc.render_pass = rp;
						PSODesc.subpass = 0;
						rp->CreatePipeline("asset/shaders/vert.spv", "asset/shaders/frag.spv", PSODesc);

						// Setup Dependencies between render passes
					},
					[](RenderPass& rp, RenderTarget& rt)
					{
						rhi::RHICommands::BeginRenderPass(rp, rt);
						rhi::RHICommands::BindGfxPipeline(rp.GetPipeline(0));
						rhi::RHICommands::SetViewport(0, 0, 1000, 800);
						rhi::RHICommands::SetScissor(0, 0, 1000, 800);
						rhi::RHICommands::Draw(3, 1);
						rhi::RHICommands::EndRenderPass();
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
					[](RenderPass& rp, RenderTarget& rt)
					{
						// Rendering
						ImGui::Render();
						ImDrawData* main_draw_data = ImGui::GetDrawData();
						const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
						if (!main_is_minimized)
						{
							rhi::RHICommands::BeginRenderPass(rp, rt);
							// Record dear imgui primitives into command buffer
							rhi::RHICommands::ImGui_ImplMLE_RenderDrawData(main_draw_data);
							rhi::RHICommands::EndRenderPass();
						}
						ImGuiIO& io = ImGui::GetIO(); (void)io;
						if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
						{
							ImGui::UpdatePlatformWindows();
							ImGui::RenderPlatformWindowsDefault();
						}
					});
			}
		}
	}

	void EditorLayer::OnDetach() 
	{
		renderer::RenderGraph& render_graph_ = renderer::RenderGraph::GetInstance();
		render_graph_.Clear();
	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		// Resize
		//if ( viewport_size_.x > 0.0f && viewport_size_.y > 0.0f && // zero sized framebuffer is invalid
		//	(spec.Width != viewport_size_.x || spec.Height != viewport_size_.y))
		//{
		//	back_buffer_->Resize((uint32_t)viewport_size_.x, (uint32_t)viewport_size_.y);
		//}
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
		ImGui::End();

		ImGui::Begin("Panel 2");
		
		ImGui::End();
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		/*m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };*/

		ImGui::Image((ImTextureID)back_buffer_->GetTextureID(), ImVec2{ (float)back_buffer_->GetWidth(), (float)back_buffer_->GetHeight()}, ImVec2{0, 1}, ImVec2{1, 0});

		ImGui::End();
		ImGui::PopStyleVar();
	}
}