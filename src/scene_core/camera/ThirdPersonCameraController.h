#pragma once
#include "CameraController.h"
#include "scene_core/ecs/Entity.h"

class ThirdPersonCameraController : public CameraController
{
private:
	Entity m_SelectedEntity;
	glm::vec3 m_Pivot{0.0f};
	float m_Radius = 5.0f;
	float m_MouseSensitivity = 0.1f;

	float m_YawOffsetDeg = 0.0f;
	float m_PitchOffsetDeg = 15.0f;

public:
	ThirdPersonCameraController() = default;

	// void SetPosition(const glm::vec3& p) { m_SelectedPos = p; }
	// const glm::vec3 GetPosition() const { return m_SelectedPos; }

	void SetEntity(const Entity& e) { m_SelectedEntity = e; }
	const Entity GetEntity() const { return m_SelectedEntity; }

	void SetPivot(const glm::vec3& pivot) { m_Pivot = pivot; }
	void SetRadius(const float& radius )  { m_Radius = radius; }


	void Update(float dt, Input& input) override;
	void OnActivate(Input& input) override;
	void OnSelect(const glm::vec3& position, float radius, Entity entity)
	{
		SetEntity(entity);
		m_Radius = radius;

		glm::vec3 target = position;
		m_Pivot = target - entity.GetComponent<TransformComponent>().GetPosition(); // usually becomes 0

		glm::vec3 camPos = m_Camera->GetPosition();
		glm::vec3 v = camPos - target;

		float len2 = glm::dot(v, v);
		if (len2 < 1e-8f)
		{
			m_YawOffsetDeg = 0.0f;
			m_PitchOffsetDeg = 15.0f;
			return;
		}

		glm::vec3 dir = v / sqrt(len2);

		float yawRad   = std::atan2(dir.z, dir.x);
		float pitchRad = std::asin(glm::clamp(dir.y, -1.0f, 1.0f));

		m_YawOffsetDeg   = glm::degrees(yawRad);
		m_PitchOffsetDeg = glm::degrees(pitchRad);
	}
};

