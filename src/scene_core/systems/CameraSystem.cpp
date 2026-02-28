#include "CameraSystem.h"
#include "scene_core/ecs/EntityUtils.h"


CameraSystem::CameraSystem(Scene* scene)
	: m_Scene(scene)
{
	m_Controllers.push_back(CreateScope<FreeCameraController>());
	m_Controllers.push_back(CreateScope<OrbitCameraController>(10.0f));
	m_Controllers.push_back(CreateScope<ThirdPersonCameraController>());
	m_ActiveController = FREE_CONTROLLER_INDEX;
}

Entity CameraSystem::GetPrimaryCamera()
{
	auto& registry = m_Scene->GetRegistry();
	auto view = registry.view<CameraComponent>();

	for (auto e : view)
	{
		auto& cc = view.get<CameraComponent>(e);
		if (cc.Primary)
			return Entity{ e, &registry };
	}

	return {};
}


void CameraSystem::Update(float dt,
						  Input& input,
						  FrameContext& ctx,
						  Scene* scene)
{
	auto& registry = m_Scene->GetRegistry();
	auto view = registry.view<CameraComponent>();

	Entity camEntity;

	for (auto e : view)
	{
		if (view.get<CameraComponent>(e).Primary)
		{
			camEntity = Entity{ e, &registry };
			break;
		}
	}

	if (!camEntity)
		return;

	auto& cc = camEntity.GetComponent<CameraComponent>();
	auto& window = m_Scene->GetWindow();

	cc.Camera.SetAspectRatio(window.GetRenderAspect());

	// -------------------------------------------------
	// Orbit Toggle Logic
	// -------------------------------------------------


	if (input.IsKeyPressedOnce(Key::P) ||
		input.IsMousePressedOnce(Mouse::Middle))
	{
		if (m_OrbitEntity != entt::entity(ctx.LastHit.entity))
		{
			m_OrbitEntity = entt::entity(ctx.LastHit.entity);
			m_OrbitEntityChanged = true;
		}
		else
			m_OrbitEntityChanged = false;


		if (m_ActiveController == ORBIT_CONTROLLER_INDEX && (!ctx.HasHit || !m_OrbitEntityChanged))
		{
			m_ActiveController = FREE_CONTROLLER_INDEX;
			m_Controllers[m_ActiveController]->OnActivate(input);
		}
		else if (ctx.HasHit)
		{
			Entity entity{ ctx.LastHit.entity, &registry };

			float radius = EntityUtils::ComputeEntityRadius(*scene, entity);

			float viewportCoverage = 0.5f;
			float fovRadians =
				glm::radians(cc.Camera.GetFOV() * 0.5f);

			float orbitDistance =
				radius / (glm::tan(fovRadians) * viewportCoverage);

			orbitDistance =
				glm::clamp(orbitDistance, 0.01f, 50000.0f);

			m_ActiveController = ORBIT_CONTROLLER_INDEX;
			auto* orb = static_cast<OrbitCameraController*>(
				m_Controllers[m_ActiveController].get());

			if (entity.HasComponent<ModelPartComponent>())
			{
				const auto root = EntityUtils::FindModelRootFromPart(*scene, entity);
				orb->OnSelect(root.GetPosition(),
							  orbitDistance,
							  root);
			}
			else
			{
				orb->OnSelect(entity.GetPosition(),
						      orbitDistance,
							  entity);
			}
		}
	}

	else if (input.IsKeyPressedOnce(Key::Y) && ctx.HasHit)
	{
		Entity entity{ ctx.LastHit.entity, &registry };
		auto& tc = entity.GetComponent<TransformComponent>();

		float radius = EntityUtils::ComputeEntityRadius(*scene, entity);

		float viewportCoverage = 0.3f;
		float fovRadians =
			glm::radians(cc.Camera.GetFOV() * 0.3f);

		float cameraDistance =
			radius / (glm::tan(fovRadians) * viewportCoverage);

		cameraDistance =
			glm::clamp(cameraDistance, 0.01f, 50000.0f);

		m_ActiveController = THIRD_PERSON_CONTROLLER_INDEX;

		auto* tp = static_cast<ThirdPersonCameraController*>(
			m_Controllers[m_ActiveController].get());

		tp->SetCamera(cc.Camera);
		tp->SetMoveSpeed(cc.MoveSpeed);
		tp->OnSelect(tc.Translation,
					 cameraDistance,
					 entity);
	}



	auto& controller =
		*m_Controllers[m_ActiveController];

	controller.SetCamera(cc.Camera);
	controller.SetMoveSpeed(cc.MoveSpeed);
	controller.Update(dt, input);

}