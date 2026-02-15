#pragma once
#include <entt/entt.hpp>

#include "FrameContext.h"
#include "scene_core/ecs/Entity.h"
#include "scene_core/camera/FreeCameraController.h"
#include "scene_core/camera/OrbitCameraController.h"
#include "core/Window.h"


class Input;
class Scene;


class CameraSystem
{
public:
	static constexpr int FREE_CONTROLLER_INDEX  = 0;
	static constexpr int ORBIT_CONTROLLER_INDEX = 1;

public:
	explicit CameraSystem(Scene* scene);

	void Update(float dt, Input& input, FrameContext& ctx, Scene* scene);

	Entity GetPrimaryCamera();

	void SetActiveController(size_t index) { m_ActiveController = index; }

	size_t GetActiveController() const { return m_ActiveController; }

private:
	Scene* m_Scene = nullptr;

	std::vector<Scope<CameraController>> m_Controllers;
	size_t m_ActiveController = 0;

	entt::entity m_OrbitEntity = entt::null;
	bool m_OrbitEntityChanged = false;


};
