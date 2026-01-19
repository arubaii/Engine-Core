#include "Scene.h"

#include "Application.h"
#include "Application.h"
#include "scene_core/Entity.h"
#include "utils/Log.h"
#include "utils/Primitives.h"
#include "io/MouseCodes.h"




// ========================================================================================= //
// ====================================== SCENE LOGIC ====================================== //
// ========================================================================================= //



Scene::Scene(Window& window, Input& input)
	: m_Window(window),
	  m_Viewport(m_Window.GetViewport()),
	  m_CameraProps{
		  45.0f,
		  window.GetAspectRatio(),
		  0.1f,
		  100000.0f
	  }
{
	m_CameraControllers.push_back(CreateScope<FreeCameraController>());
	m_CameraControllers.push_back(CreateScope<OrbitCameraController>(10.0f));
	m_ActiveController = FREE_CONTROLLER_INDEX;

	{
		Entity camera = CreateEntity(UUID(), "Main Camera");
		auto& cc = camera.AddComponent<CameraComponent>(
			m_CameraProps.Fov,
			m_CameraProps.AspectRatio,
			m_CameraProps.NearPlane,
			m_CameraProps.FarPlane
		);
		cc.Primary = true;

		cc.Camera.SetPosition(DefaultCameraPosition);
		cc.Camera.SetRotation(DefaultPitch, DefaultYaw);
		cc.Camera.RecalculateView();
		m_CameraControllers[m_ActiveController]->OnActivate(input);

		Entity light = CreateEntity(UUID(), "Light");

		auto& lightComponent = light.AddComponent<MeshComponent>();
		lightComponent.MeshData = CreateRef<Mesh>();
		lightComponent.MeshData->Vertices = PRIMITIVES::LightVerts;
		lightComponent.MeshData->Indices  = PRIMITIVES::LightIdx;

		light.GetComponent<TransformComponent>().Scale = {25.0f, 25.0f, 25.0f};
		light.GetComponent<TransformComponent>().Translation = {5.0f, -2.5f, 10.0f};

		Entity yAxis = CreateEntity(UUID(), "Y-Axis");
		auto& yAxisComponent = yAxis.AddComponent<MeshComponent>();
		yAxisComponent.MeshData = CreateRef<Mesh>();
		yAxisComponent.MeshData->Vertices = PRIMITIVES::GetyAxisVertices(m_CameraProps.FarPlane);
		yAxisComponent.MeshData->Indices = PRIMITIVES::yAxisIndices;

		yAxis.GetComponent<TransformComponent>().Translation = {0.0f, -2.5f, 0.0f};
	}

	m_GridShader  	= Shader::Create("infinite_grid.vert", "infinite_grid.frag");
	m_LightShader 	= Shader::Create("light.vert", "light.frag");
	m_BaseShader  	= Shader::Create("base.vert", "base.frag");
	m_PhongShader 	= Shader::Create("phong.vert", "phong.frag");
	m_OutlineShader = Shader::Create("outline.vert", "outline.frag");

	m_CrosshairShader = Shader::Create("crosshair.vert", "crosshair.frag");
	m_CrosshairLayout.Push<float>(3);
	m_CrosshairVB = VertexBuffer(sizeof(PRIMITIVES::CrosshairVertices), PRIMITIVES::CrosshairVertices);
	m_CrosshairVA.AddBuffer(m_CrosshairVB, m_CrosshairLayout);
	glGenVertexArrays(1, &m_GridVAO);
}

void Scene::Render(Renderer& renderer)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	Entity cam = GetPrimaryCameraEntity();
	if (!cam) return;
	auto& cc = cam.GetComponent<CameraComponent>();

	if (m_ShowGrid)
		DrawGrid(cc);

	glm::mat4 VP =
		cc.Camera.GetProjectionMatrix() *
		cc.Camera.GetViewMatrix();

	glm::vec3 lightPos{0.0f};
	glm::vec4 lightColor{1.0f};
	glm::mat4 lightModel{1.0f};

	auto lightView = m_Registry.view<TransformComponent, TagComponent>();
	for (auto e : lightView)
	{
		auto& tag = lightView.get<TagComponent>(e);
		if (tag.Tag == "Light")
		{
			auto& tc = lightView.get<TransformComponent>(e);
			lightModel = tc.GetTransform();
			lightPos   = glm::vec3(lightModel[3]);
			break;
		}
	}

	auto view = m_Registry.view<TransformComponent, MeshComponent, TagComponent>();
	for (auto e : view)
	{
		auto& tag = m_Registry.get<TagComponent>(e);
		auto& tc  = m_Registry.get<TransformComponent>(e);
		auto& mc  = m_Registry.get<MeshComponent>(e);

		if (!mc.MeshData)
			continue;

		GPUMesh& gpu = MeshRendererCache::GetOrCreate(*mc.MeshData);

		glm::mat4 model = tc.GetTransform();
		glm::mat4 MVP   = VP * model;

		if (tag.Tag == "Light")
		{
			m_LightShader->Bind();
			renderer.SetShader(m_LightShader);
			m_LightShader->SetMat4("u_MVP", MVP);
			m_LightShader->SetVec4("u_LightColor", lightColor);
			renderer.Draw(gpu.VA, gpu.IB);
			if (m_IsDragging)
			{
				renderer.DrawOutline(m_OutlineShader, MVP, gpu.VA, gpu.IB);
			}
		}
		else if (tag.Tag == "Y-Axis" && m_ShowGrid)
		{
			m_BaseShader->Bind();
			renderer.SetShader(m_BaseShader);
			m_BaseShader->SetMat4("u_MVP", MVP);
			m_BaseShader->SetMat4("u_Model", model);
			renderer.DrawLines(gpu.VA, gpu.IB);
		}
		else
		{
			m_PhongShader->Bind();
			renderer.SetShader(m_PhongShader);
			m_PhongShader->SetPhongUniforms(
				model,
				cc.Camera.GetProjectionMatrix(),
				lightPos,
				lightColor,
				cc.Camera
			);
			renderer.Draw(gpu.VA, gpu.IB);

			if (m_IsDragging)
			{
				renderer.DrawOutline(m_OutlineShader, MVP, gpu.VA, gpu.IB);
			}
		}
	}

	DrawScreenOverlays(cc, renderer);
}


void Scene::Update(float dt, Input& input)
{
	if (m_CameraControllers.empty())
		return;

	if (m_ActiveController >= m_CameraControllers.size())
		m_ActiveController = 0;

	Entity camEntity = GetPrimaryCameraEntity();
	if (!camEntity)
		return;

	auto& cc = camEntity.GetComponent<CameraComponent>();

	auto& controller = *m_CameraControllers[m_ActiveController];
	controller.SetCamera(cc.Camera);
	controller.Update(dt, input);

	glm::vec2 mousePos = input.GetMousePos();
	glm::vec2 viewport = m_Window.GetViewport();
	glm::vec2 center = viewport * 0.5f;

	Ray ray;
	if (input.IsCursorEnabled())
		ray = cc.Camera.GetRayFromScreen(input.GetMousePos(), viewport);
	else
		ray = cc.Camera.GetRayFromScreen(center, viewport);

	RayHit hit;
	bool hasHit = Raycast(ray, hit) && hit.entity != entt::null;

	if (input.IsMousePressedOnce(Mouse::Middle))
	{
		if (m_ActiveController == ORBIT_CONTROLLER_INDEX)
			m_ActiveController = FREE_CONTROLLER_INDEX;
		else if (hasHit)
		{
			CROSSHAIR_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);
			Entity entity{ hit.entity, this };

			m_ActiveController = ORBIT_CONTROLLER_INDEX;
			m_CameraControllers[m_ActiveController]->OnSelect(entity.GetPosition());
		}
	}

	if (hasHit)
		CROSSHAIR_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);
	else
		CROSSHAIR_COLOR = glm::vec3(1.0f);

	if (!m_IsDragging &&
		input.IsMousePressedOnce(Mouse::Left) &&
		m_ActiveController == FREE_CONTROLLER_INDEX &&
		hasHit)
	{
		m_DraggedEntity = hit.entity;
		m_IsDragging = true;

		Entity dragged{ m_DraggedEntity, this };
		m_DragOffset = dragged.GetPosition() - hit.position;
	}

	constexpr float MAX_DRAG_DISTANCE = 250.0f;
	if (m_IsDragging &&
		input.IsMousePressed(Mouse::Left) &&
		m_DraggedEntity != entt::null)
	{
		glm::vec3 planeHit;
		if (IntersectPlane(ray, 0.0f, planeHit))
		{
			Entity dragged{ m_DraggedEntity, this };

			glm::vec3 desiredPos = planeHit + m_DragOffset;

			const glm::vec3 camPos = cc.Camera.GetPosition();
			glm::vec3 camToPos = desiredPos - camPos;

			float dist = glm::length(camToPos);
			if (dist > MAX_DRAG_DISTANCE)
				camToPos = glm::normalize(camToPos) * MAX_DRAG_DISTANCE;

			dragged.GetComponent<TransformComponent>().Translation =
				camPos + camToPos;
		}
	}

	if (m_IsDragging && !input.IsMousePressed(Mouse::Left))
	{
		m_IsDragging = false;
		m_DraggedEntity = entt::null;
		m_DragOffset = glm::vec3(0.0f);
	}
}

void Scene::DrawScreenOverlays(const CameraComponent& cc, Renderer& renderer)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glDepthMask(GL_TRUE);

	m_CrosshairShader->Bind();
	m_CrosshairShader->SetVec3("u_Color", CROSSHAIR_COLOR);
	m_CrosshairVA.Bind();
	glLineWidth(10.0f);
	glDrawArrays(GL_LINES, 0, 4);
}

void Scene::DrawGrid(const CameraComponent& cc) const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glBindVertexArray(m_GridVAO);

	m_GridShader->Bind();
	m_GridShader->SetFloat("GridHeight", 0.0f);
	m_GridShader->SetMat4("Projection", cc.Camera.GetProjectionMatrix());
	m_GridShader->SetMat4("View", cc.Camera.GetViewMatrix());
	m_GridShader->SetVec3("CameraWorldPos", cc.Camera.GetPosition());
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

bool Scene::Raycast(const Ray& ray, RayHit& outHit) const
{
	bool hitAnything = false;

	auto view = m_Registry.view<TransformComponent, MeshComponent>();
	for (auto e : view)
	{
		const auto& tc = view.get<TransformComponent>(e);
		const auto& mc = view.get<MeshComponent>(e);

		if (!mc.MeshData)
			continue;

		const glm::mat4 model = tc.GetTransform();

		const auto& vertices = mc.MeshData->Vertices;
		const auto& indices  = mc.MeshData->Indices;

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			glm::vec3 v0 = glm::vec3(model * glm::vec4(vertices[indices[i+0]].Position, 1.0f));
			glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[indices[i+1]].Position, 1.0f));
			glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[indices[i+2]].Position, 1.0f));

			float t;
			glm::vec3 normal;

			if (RayIntersectsTriangle(ray, v0, v1, v2, t, normal))
			{
				if (t < outHit.t)
				{
					outHit.Hit      = true;
					outHit.t        = t;
					outHit.position = ray.At(t);
					outHit.normal   = normal;
					outHit.entity   = e;
					hitAnything     = true;
				}
			}
		}
	}

	return hitAnything;
}




// ========================================================================================= //
// ======================================= ECS LOGIC ======================================= //
// ========================================================================================= //




void Scene::SetPrimaryCamera(Entity entity)
{
	auto view = m_Registry.view<CameraComponent>();

	for (auto e : view)
		view.get<CameraComponent>(e).Primary = false;

	entity.GetComponent<CameraComponent>().Primary = true;
}

Entity Scene::GetPrimaryCameraEntity()
{
	auto view = m_Registry.view<CameraComponent>();
	for (auto e : view)
	{
		auto& component = view.get<CameraComponent>(e);
		if (component.Primary)
			return Entity{e, this};
	}

	return {};
}

glm::vec3 Scene::GetMainCameraPos()
{
	Entity camEntity = GetPrimaryCameraEntity();
	auto& cc = camEntity.GetComponent<CameraComponent>();
	return cc.Camera.GetPosition();
}

float Scene::GetMainCameraPitch()
{
	Entity camEntity = GetPrimaryCameraEntity();
	auto& cc = camEntity.GetComponent<CameraComponent>();
	return cc.Camera.GetPitch();
}

float Scene::GetMainCameraYaw()
{
	Entity camEntity = GetPrimaryCameraEntity();
	auto& cc = camEntity.GetComponent<CameraComponent>();
	return cc.Camera.GetYaw();
}

Entity Scene::InitEntity(const std::string& name)
{
	return CreateEntity(UUID(), name);
}

Entity Scene::CreateEntity(UUID uuid, const std::string& name)
{
	Entity entity = { m_Registry.create(), this };
	entity.AddComponent<IDComponent>(uuid);
	entity.AddComponent<TransformComponent>();
	auto& tag = entity.AddComponent<TagComponent>();
	tag.Tag = name.empty() ? "Entity" : name;

	m_EntityMap[uuid] = entity;
	return entity;
}

void Scene::DeleteEntity(Entity entity)
{
	m_EntityMap.erase(entity.GetUUID());
	m_Registry.destroy(entity);
}

template<typename T>
void Scene::OnComponentAdded(Entity, T&) {}

template<>
void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
{
	if (component.Primary)
		SetPrimaryCamera(entity);
}