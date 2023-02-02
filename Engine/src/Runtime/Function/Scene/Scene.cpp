#include "mlepch.h"
#include "Scene.h"
#include "Entity.h"

namespace engine {
	Entity Scene::CreateEntity(const char* name)
	{
		Entity entity = { registry_.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name ? name : "Entity";
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		registry_.destroy(entity);
	}
}