#include "mlepch.h"
#include "SceneHierachyPanel.h"

#include <imgui.h>

namespace editor {
	SceneHierachyPanel::SceneHierachyPanel(const std::shared_ptr<engine::Scene>& in_scene)
	{
		SetScene(in_scene);
	}

	void SceneHierachyPanel::SetScene(const std::shared_ptr<engine::Scene>& in_scene)
	{
		current_scene_ = in_scene;
		selected_entity_ = {};
	}

	void SceneHierachyPanel::OnUIRender()
	{
		ImGui::Begin("Scene");

		if (current_scene_)
		{
			current_scene_->ForEachEntity([&](auto entity_handle) {
				engine::Entity entity{ entity_handle, current_scene_.get() };

				auto& tag = entity.GetComponent<engine::TagComponent>().tag;

				ImGuiTreeNodeFlags flags = ((selected_entity_ == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
				flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
				bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
				if (ImGui::IsItemClicked())
				{
					selected_entity_ = entity;
				}

				bool entity_deleted = false;
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Delete Entity"))
						entity_deleted = true;

					ImGui::EndPopup();
				}

				if (opened)
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
					bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
					if (opened)
						ImGui::TreePop();
					ImGui::TreePop();
				}

				if (entity_deleted)
				{
					current_scene_->DestroyEntity(entity);
					if (selected_entity_ == entity)
						selected_entity_ = {};
				}
				});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				selected_entity_ = {};

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1, false))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					current_scene_->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}
		}

		ImGui::End();

		ImGui::Separator();

		ImGui::Begin("Properties");

		if (selected_entity_)
		{
			if (selected_entity_.HasComponent<engine::TagComponent>())
			{
				auto& tag = selected_entity_.GetComponent<engine::TagComponent>().tag;

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					tag = std::string(buffer);
				}
			}
		}

		ImGui::End();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, engine::Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar(
			);
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}
}