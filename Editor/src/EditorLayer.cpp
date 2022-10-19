#include "mlepch.h"
#include "EditorLayer.h"
#include "Runtime/Core/Base/Layer.h"
#include "Runtime/Core/Base/Application.h"
#include "Runtime/Function/RHI/RHICommands.h"

#include "backends/imgui_impl_vulkan.h"

namespace editor {
	void EditorLayer::OnAttach()
	{
		//frame_buffer_ = rhi::RHITexture2D::Create(1280, 720, rhi::PixelFormat::RGBA);
		renderer::RenderGraph& render_graph_ = renderer::RenderGraph::GetInstance();
		renderer::RenderPassDesc test_pass{};
		test_pass.is_for_present = true;
		render_graph_.AddRenderPass("TEST", test_pass,
			[=](auto rp)
			{

			},
			[=](auto rp)
			{
				// Rendering
				ImGui::Render();
				ImDrawData* main_draw_data = ImGui::GetDrawData();
				const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
				if (!main_is_minimized)
				{
					rhi::RHICommands::BeginRenderPass(*rp);
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

	void EditorLayer::OnDetach() 
	{
		renderer::RenderGraph& render_graph_ = renderer::RenderGraph::GetInstance();
		render_graph_.Clear();
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
					/*NewScene();*/

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					//OpenScene();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					//SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					//SaveSceneAs();

				if (ImGui::MenuItem("Exit")) 
					engine::Application::GetApp().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::ShowDemoWindow();
		ImGui::End();

		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();
	}
}