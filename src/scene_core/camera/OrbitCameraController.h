#pragma once
#include "CameraController.h"
#include "scene_core/ecs/Entity.h"


class OrbitCameraController : public CameraController
{
private:
	glm::vec3 m_Pivot{0.0f};
	float m_Radius = 0.0f;
	float m_Yaw    = 0.0f;
	float m_Pitch  = 0.0f;

	Entity m_SelectedEntity;

	// TODO: Add serialization for changes in settings in GUI
	float m_MouseSensitivity       = 0.075f;
	float m_MouseScrollSensitivity = 0.002f;
	float m_KeySensitivity	       = 135.0f; // 135 deg per second
	float m_KeyZoomSensitivity     = 10.0f;
	float m_MinRadius              = 5.0f;
	float m_MaxRadius		       = 100000.0f;
	float m_ZoomVelocity		   = 0.0f;

	glm::vec2 m_AngularVelocity{};
	glm::vec2 m_EntityAngularVelocity{};

public:
	explicit OrbitCameraController(float radius = 10.0f)
	: m_Radius(radius) {}

	void SetPivot(const glm::vec3& pivot) { m_Pivot = pivot; }
	void SetRadius(const float& radius )  { m_Radius = radius; }
	void SetEntity(const Entity& e) { m_SelectedEntity = e; }

	void Update(float dt, Input& input) override;
	void OnActivate(Input& input) override;		// Reset mouse deltas

	void OnSelect(const glm::vec3& position, float radius, const Entity& e)
	{
		SetPivot(position);
		SetRadius(radius);
		SetEntity(e);
	}


};
