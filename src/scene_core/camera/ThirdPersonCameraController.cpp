#include "ThirdPersonCameraController.h"

#include "scene_core/ecs/EntityUtils.h"



void ThirdPersonCameraController::Update(float dt, Input& input)
{
	assert(m_Camera && "Camera not bound to ThirdPersonCameraController");

	auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();

	const float velocity = m_MoveSpeed * dt;

	glm::vec3 move{0.0f};

	glm::vec3 fwd = EntityUtils::GetEntityForward(tc);
	if (glm::length2(fwd) < 1e-8f)
		fwd = glm::vec3(0.0f, 0.0f, -1.0f);
	fwd = glm::normalize(fwd);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(fwd, up));

	if (input.IsKeyboardEnabled())
	{
		if (input.IsActionActive(InputAction::MoveForward))  move += fwd;
		if (input.IsActionActive(InputAction::MoveBackward)) move -= fwd;
		if (input.IsActionActive(InputAction::MoveLeft)) 	 move -= right;
		if (input.IsActionActive(InputAction::MoveRight))	 move += right;
		if (input.IsActionActive(InputAction::MoveUp))       move += up;
		if (input.IsActionActive(InputAction::MoveDown))     move -= up;
	}

	if (move != glm::vec3(0.0f))
	{
		move = glm::normalize(move);
		tc.SetTranslation(tc.GetPosition() + move * velocity);
	}

	glm::vec3 target = tc.GetPosition() + m_Pivot;

	if (!input.IsCursorEnabled())
	{
		glm::vec2 mouse = input.GetMouseDelta();

		m_YawOffsetDeg   += mouse.x * m_MouseSensitivity;
		m_PitchOffsetDeg -= mouse.y * m_MouseSensitivity;

		m_PitchOffsetDeg = std::clamp(m_PitchOffsetDeg, -80.0f, 80.0f);
	}

	float yawRad   = glm::radians(m_YawOffsetDeg);
	float pitchRad = glm::radians(m_PitchOffsetDeg);

	glm::vec3 orbitDir;
	orbitDir.x = cos(pitchRad) * cos(yawRad);
	orbitDir.y = sin(pitchRad);
	orbitDir.z = cos(pitchRad) * sin(yawRad);

	orbitDir = glm::normalize(orbitDir);

	glm::vec3 cameraPos = target - orbitDir * m_Radius;

	m_Camera->SetPosition(cameraPos);
	m_Camera->LookAt(target);
	m_Camera->RecalculateView();
}

void ThirdPersonCameraController::OnActivate(Input& input)
{
	// Reset mouse state so first frame doesn't jump
	input.GetMouseDelta();
}
