#pragma once

#include "entt/entt.hpp"
#include "Scene.h"
#include "Components.h"

namespace engine {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* in_scene)
			:belonged_scene_(in_scene), entity_handle_(handle) {};

		template<typename COMPONENT, typename... Args>
		COMPONENT& AddComponent(Args&&... args)
		{
			assert(!HasComponent<COMPONENT>() && "Entity already has component");
			COMPONENT& component = belonged_scene_->registry_.emplace<COMPONENT>(entity_handle_, std::forward<Args>(args)...);

			return component;
		}

		template<typename COMPONENT>
		COMPONENT& GetComponent()
		{
			assert(HasComponent<COMPONENT>() && "Entity dose not have component");
			return belonged_scene_->registry_.get<COMPONENT>(entity_handle_);
		}

		template<typename COMPONENT>
		bool HasComponent()
		{
			return belonged_scene_->registry_.all_of<COMPONENT>(entity_handle_);
		}

		template<typename COMPONENT>
		void RemoveComponent()
		{
			assert(HasComponent<COMPONENT>() && "Entity dose not have component");
			belonged_scene_->registry_.remove<COMPONENT>(entity_handle_);
		}

		operator bool() const { return entity_handle_ != entt::null; }
		operator entt::entity() const { return entity_handle_; }
		operator uint32_t() const { return (uint32_t)entity_handle_; }

		bool operator==(const Entity& other) const
		{
			return entity_handle_ == other.entity_handle_ && belonged_scene_ == other.belonged_scene_;
		}


	private:
		Scene* belonged_scene_ = nullptr;
		entt::entity entity_handle_{ entt::null };
	 };
}