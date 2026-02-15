#pragma once
#include "FrameContext.h"
#include "scene_core/ecs/Components.h"
#include "core/Window.h"

class RaycastSystem
{
public:
	RaycastSystem(entt::registry& registry);

	void Update(Input& input,
				CameraComponent& camera,
				Window& window,
				FrameContext& ctx);

private:
	entt::registry& m_Registry;


	bool Raycast(const Ray& ray, RayHit& outHit);
	bool RayIntersectsSphere(const Ray& ray, const glm::vec3& center, float radius, float& tOut);
};

