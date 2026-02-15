#include "OrbitCameraController.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/Log.h"


void OrbitCameraController::Update(float dt, Input& input)
{
	assert(m_Camera && "Camera not bound to FreeCameraController");

	glm::vec2 mouse    = input.GetMouseDelta();
	double mouseScroll = input.GetMouseScrollY();


	// TODO: Set min and max radius proportional to the size the pivot object

	if (input.IsMousePressed(Mouse::Left) && !input.IsInUI())
	{
		m_Yaw    += mouse.x * m_MouseSensitivity;
		m_Pitch  -= mouse.y * m_MouseSensitivity;
	}

	// Zoom
	if (mouseScroll != 0.0 && !input.IsInUI())
	{

		m_Radius *= std::exp(-mouseScroll * m_MouseScrollSensitivity);
	}

	if (!input.IsKeyboardCapturedByUI())
	{
		if (input.IsKeyPressed(Key::A) || input.IsKeyPressed(Key::Left))  m_Yaw   += m_KeySensitivity * dt;
		if (input.IsKeyPressed(Key::D) || input.IsKeyPressed(Key::Right)) m_Yaw   -= m_KeySensitivity * dt;
		if (input.IsKeyPressed(Key::W) || input.IsKeyPressed(Key::Up))    m_Pitch += m_KeySensitivity * dt;
		if (input.IsKeyPressed(Key::S) || input.IsKeyPressed(Key::Down))  m_Pitch -= m_KeySensitivity * dt;
		if (input.IsKeyPressed(Key::Space))    m_Radius *= std::exp(-m_KeyZoomSensitivity * dt);
		if (input.IsKeyPressed(Key::C))		   m_Radius *= std::exp(+m_KeyZoomSensitivity * dt);
	}

	// Clamp radius
	if (m_Radius < m_MinRadius)
		m_Radius = m_MinRadius + 0.001f; // Small buffer
	if (m_Radius > m_MaxRadius)
		m_Radius = m_MaxRadius - 0.001f;

	// Prevent flipping
	m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

	// Spherical -> Cartesian
	glm::vec3 offset;
	offset.x = m_Radius * cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
	offset.y = m_Radius * sin(glm::radians(m_Pitch));
	offset.z = m_Radius * cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));

	glm::vec3 position = m_Pivot + offset;

	m_Camera->SetPosition(position);
	m_Camera->SetRotation(-m_Pitch, m_Yaw + 180.0f);
	m_Camera->RecalculateView();

}


void OrbitCameraController::OnActivate(Input& input)
{
	// Recompute yaw/pitch from camera -> pivot direction
	glm::vec3 dir = glm::normalize(m_Camera->GetPosition() - m_Pivot);

	m_Yaw   = glm::degrees(atan2(dir.z, dir.x));
	m_Pitch = glm::degrees(asin(dir.y));

	// Lock radius to current distance
	m_Radius = glm::length(m_Camera->GetPosition() - m_Pivot);

	input.GetMouseDelta();
}
