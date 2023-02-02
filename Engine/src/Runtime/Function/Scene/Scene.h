#pragma once
#include "entt/entt.hpp"
#include "Runtime/Resource/Vertex.h"

namespace engine {
	class Entity;


	class Scene
	{
		friend class Entity;
		friend class EditorLayer;

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

		// TEMP: still not sure whether to keep this function or not
		template<typename Func>
		inline void ForEachEntity(Func func)
		{
			registry_.each(std::forward<Func>(func));
		}
	private:
		entt::registry registry_;
	};
}