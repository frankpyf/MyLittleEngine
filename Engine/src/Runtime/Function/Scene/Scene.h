#pragma once
#include "entt/entt.hpp"

namespace engine {
	class Entity;

	class Scene
	{
		friend class Entity;

	public:
		Scene() = default;
		~Scene() = default;

		Entity CreateEntity(const char* name = nullptr);
		void DestroyEntity(Entity entity);

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return registry_.view<Components...>();
		}

	private:
		entt::registry registry_;

	};
}