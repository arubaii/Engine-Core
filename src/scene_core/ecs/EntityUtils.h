#pragma once

#include "core/Scene.h"
#include "scene_core/ecs/Entity.h"

struct EntityContext
{
	entt::registry* Registry = nullptr;

	std::unordered_map<UUID, entt::entity>* IDMap = nullptr;
	std::unordered_map<std::string, entt::entity>* NameMap = nullptr;
};

namespace EntityUtils
{

	size_t GetMaxEntityIndex(Scene& scene, std::string startsWith);

	Entity DuplicateEntity(Scene& scene, Entity src);

	void SetEntityMaterial(Scene& scene, Entity e, AssetHandle matHandle);

	void SetModelMaterial(Scene& scene, Entity root, AssetHandle matHandle);

	void BindMaterial(Scene& scene, const MaterialComponent& material, const Ref<Shader>& shader, int& slot);

	void DeleteSnapshotRecursive(Scene* scene, const EntitySnapshot& s);

	Entity FindModelRootFromPart(Entity part);

	glm::vec3 ColorFromTemperature(float kelvin);

	float ComputeEntityFloorY(Scene* scene, Entity e);

	float ComputeXZRadius(Scene* scene, Entity e);

	float ComputeEntityRadius(Scene& scene, Entity entity);

	bool SnapshotContainsUUID(const EntitySnapshot& s, UUID id);

}