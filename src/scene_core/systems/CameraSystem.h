#pragma once
#include <entt/entt.hpp>

#include "FrameContext.h"
#include "scene_core/ecs/Entity.h"
#include "scene_core/camera/FreeCameraController.h"
#include "scene_core/camera/OrbitCameraController.h"
#include "scene_core/camera/ThirdPersonCameraController.h"
#include "core/Window.h"


class Input;
class Scene;


class CameraSystem
{
public:
	static constexpr int FREE_CONTROLLER_INDEX  = 0;
	static constexpr int ORBIT_CONTROLLER_INDEX = 1;
	static constexpr int THIRD_PERSON_CONTROLLER_INDEX = 2;

public:
	explicit CameraSystem(Scene* scene);

	void Update(float dt, Input& input, FrameContext& ctx, Scene* scene);

	Entity GetPrimaryCamera();

	CameraController* GetActiveController()
	{
		if (m_Controllers.empty())
			return nullptr;
		return m_Controllers[m_ActiveController].get();
	}

	const CameraController* GetActiveController() const
	{
		if (m_Controllers.empty())
			return nullptr;
		return m_Controllers[m_ActiveController].get();
	}

	void SetActiveControllerIndex(size_t index) { m_ActiveController = index; }
	size_t GetActiveControllerIndex() const { return m_ActiveController; }

private:
	Scene* m_Scene = nullptr;

	std::vector<Scope<CameraController>> m_Controllers;
	size_t m_ActiveController = 0;

	entt::entity m_OrbitEntity = entt::null;
	bool m_OrbitEntityChanged = false;


};
