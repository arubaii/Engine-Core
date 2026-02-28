#include "Scene.h"

#include "Application.h"
#include "Application.h"
#include "Application.h"
#include "Application.h"
#include "Application.h"
#include "asset_core/AssetManager.h"
#include "asset_io/ModelImporter.h"
#include "../scene_core/ecs/Entity.h"
#include "utils/Log.h"
#include "io/MouseCodes.h"

#include "renderer_core/GLHDRTexture2D.h"
#include "renderer_core/MeshUtils.h"
#include "scene_core/ecs/EntityUtils.h"

#include "renderer_core/Skybox.h"



Scene::Scene(Window& window, Input& input)
	: m_Window(window),
	  m_Viewport(m_Window.GetViewport()),
	  m_CameraProps{
		  45.0f,
		  window.GetRenderAspect(),
		  0.1f,
		  100000.0f,
	  	  100.0f
	  }
{


	// ==========================================================================================
	// =================================== ADD ENTITIES =========================================
	// ==========================================================================================

	{
		Entity suz = CreateEntityFromModel("../assets/models/suzanne/suzanne.glb", "Suzanne");
		auto& tcSuz = GetEntityByName("Suzanne").GetComponent<TransformComponent>();
		tcSuz.Scale *= glm::vec3(10.0f);
		tcSuz.MarkDirty();

		Entity suzSmooth = CreateEntityFromModel("../assets/models/suzanne_smooth/scene.gltf", "Suzanne Smooth");
		auto& tcSuzSmooth = GetEntityByName("Suzanne Smooth").GetComponent<TransformComponent>();
		tcSuzSmooth.Scale *= glm::vec3(10.0f);
		tcSuzSmooth.Translation += glm::vec3(35.0f, 0.0f, 0.0f);
		tcSuzSmooth.MarkDirty();

		Entity SH = CreateEntityFromModel("../assets/models/space_helmet/scene.gltf", "Space Helmet");
		auto& tcSH = GetEntityByName("Space Helmet").GetComponent<TransformComponent>();
		tcSH.Scale *= glm::vec3(9.0f);
		tcSH.Translation += glm::vec3(63.0f, 0.0f, 0.0f);
		tcSH.MarkDirty();

		Entity bunny = CreateEntityFromModel("../assets/models/stanford_bunny/scene.gltf", "Stanford Bunny");
		auto& tcBunny = GetEntityByName("Stanford Bunny").GetComponent<TransformComponent>();
		tcBunny.Scale *= glm::vec3(1.0f);
		tcBunny.Translation += glm::vec3(85.0f, 0.0f, 0.0f);
		tcBunny.MarkDirty();


		{
			Entity suzSmooth = GetEntityByName("Suzanne Smooth");
			auto& rootModel = suzSmooth.GetComponent<ModelRootComponent>();

			for (UUID id : rootModel.Parts)
			{
				Entity part = GetEntityByID(id);
				if (!part) continue;

				auto& mc = part.GetComponent<MeshComponent>();
				mc.BasisRotation.x = glm::radians(-90.0f);
			}
		}

		{
			Entity sh = GetEntityByName("Space Helmet");
			auto& rootModel = sh.GetComponent<ModelRootComponent>();

			for (UUID id : rootModel.Parts)
			{
				Entity part = GetEntityByID(id);
				if (!part) continue;

				auto& mc = part.GetComponent<MeshComponent>();
				mc.BasisRotation.x = glm::radians(90.0f);
			}
		}

		{
			Entity bunny = GetEntityByName("Stanford Bunny");
			auto& rootModel = bunny.GetComponent<ModelRootComponent>();

			for (UUID id : rootModel.Parts)
			{
				Entity part = GetEntityByID(id);
				if (!part) continue;

				auto& mc = part.GetComponent<MeshComponent>();
				mc.BasisRotation.x = glm::radians(270.0f);
			}
		}


	}

	// ==========================================================================================
	// ==========================================================================================
	// ==========================================================================================



	{
		m_CameraSystem		  = CreateScope<CameraSystem>(this);
		m_RaycastSystem		  = CreateScope<RaycastSystem>(m_Registry);
		m_SelectionSystem	  = CreateScope<SelectionSystem>(this);
		m_DragSystem		  = CreateScope<DragSystem>(this);
		m_EditorCommandSystem = CreateScope<EditorCommandSystem>(this);
		m_UndoSystem		  = CreateScope<UndoSystem>(
			m_Registry,
			[this](const EntitySnapshot& s)
			{
				return RestoreEntityFromSnapshot(s);
			},
			[this](UUID id)
			{
				Entity e = GetEntityByID(id);
				if (e)
					DeleteEntity(e);
			},
			[this](UUID id)
			{
				return GetEntityByID(id);
			}
		);


	}
	{
		Entity camera = CreateEntity(UUID(), "Main Camera");
		auto& cc = camera.AddComponent<CameraComponent>(
			m_CameraProps.Fov,
			m_CameraProps.AspectRatio,
			m_CameraProps.NearPlane,
			m_CameraProps.FarPlane,
			m_CameraProps.MoveSpeed
		);
		cc.Primary = true;

		cc.Camera.SetPosition(DefaultCameraPosition);
		cc.Camera.SetRotation(DefaultPitch, DefaultYaw);
		cc.Camera.RecalculateView();


		{
			Entity light = CreateEntity(UUID(), "Light");

			auto& lmc = light.AddComponent<MeshComponent>();
			lmc.MeshData = CreateRef<Mesh>();
			lmc.MeshData->Vertices = m_SphereMesh.Vertices;
			lmc.MeshData->Indices  = m_SphereMesh.Indices;

			Entity light2 = CreateEntity(UUID(), "Light 2");
			auto& lmc2 = light2.AddComponent<MeshComponent>();
			lmc2.MeshData = CreateRef<Mesh>();
			lmc2.MeshData->Vertices = m_SphereMesh.Vertices;
			lmc2.MeshData->Indices  = m_SphereMesh.Indices;
			{
				auto& lightComponent = light.AddComponent<LightComponent>();
				lightComponent.Luminosity = 10000.0f;
				lightComponent.TintColor = glm::vec3(1.0f);
				lightComponent.Temperature = 7000;
				lightComponent.HideLight = false;

				auto& ltc = light.GetComponent<TransformComponent>();
				ltc.Translation = {46.43f, 50.38f, 104.24f};
				ltc.Scale = glm::vec3(10.0f);
				ltc.MarkDirty();
			}
			{
				auto& lightComponent2 = light2.AddComponent<LightComponent>();
				lightComponent2.Luminosity = 10000.0f;
				lightComponent2.TintColor = glm::vec3(1.0f);
				lightComponent2.Temperature = 2500;
				lightComponent2.HideLight = true;

				auto& ltc2 = light2.GetComponent<TransformComponent>();
				ltc2.Translation = {99.74f, 39.96f, -67.25f};
				ltc2.Scale = glm::vec3(10.0f);
				ltc2.MarkDirty();
			}
		}

		Entity yAxis = CreateEntity(UUID(), "Y-Axis");
		auto& yAxisComponent = yAxis.AddComponent<MeshComponent>();
		yAxisComponent.MeshData = CreateRef<Mesh>();
		yAxisComponent.MeshData->Vertices = PRIMITIVES::GetyAxisVertices(m_CameraProps.FarPlane);
		yAxisComponent.MeshData->Indices  = PRIMITIVES::yAxisIndices;

		auto& ytc = yAxis.GetComponent<TransformComponent>();
		ytc.Translation = {0.0f, -2.5f, 0.0f};
		ytc.MarkDirty();
	}

	{

		auto genMaterialSpheres = [this](int N, glm::vec3 firstSpherePos, int stride, float scale, const char dir)
		{

			auto view = m_Registry.view<TagComponent>();
			size_t maxIndex = 0;

			for (auto e : view)
			{
				auto& tag  = m_Registry.get<TagComponent>(e);
				std::string name = tag.Tag;

				if (!name.starts_with("Sphere "))
					continue;

				std::string sphereIndex = name.substr(7); // length of "Sphere "

				// Ensure it's actually a number
				if (sphereIndex.empty() || !std::all_of(sphereIndex.begin(), sphereIndex.end(), ::isdigit))
					continue;

				size_t val = std::stoi(sphereIndex); // stoi converts a string to integer value
				maxIndex = std::max(maxIndex, val);
			}

			for (size_t i = 0; i < N; i++)
			{

				size_t index = i + maxIndex + 1;
				Entity sphere = CreateEntity(UUID(), "Sphere " + std::to_string(index));

				auto& tc = sphere.GetComponent<TransformComponent>();

				if (dir == 'x')
					tc.Translation = firstSpherePos + glm::vec3(float(stride * i), 0.0f, 0.0f);
				else if (dir == 'y')
					tc.Translation = firstSpherePos + glm::vec3(0.0f, float(stride * i), 0.0f);
				else if (dir == 'z')
					tc.Translation = firstSpherePos + glm::vec3(0.0f, 0.0f, float(stride * i));
				else
					tc.Translation = firstSpherePos + glm::vec3(float(stride * i), 0.0f, 0.0f);

				tc.Scale = glm::vec3(scale);
				tc.MarkDirty();

				auto& mc = sphere.AddComponent<MeshComponent>();
				mc.MeshData = CreateRef<Mesh>();
				mc.MeshData->Vertices = m_SphereMesh.Vertices;
				mc.MeshData->Indices  = m_SphereMesh.Indices;

				AssetHandle matHandle = AssetManager::GetHandleForPath("materials/test_pbr.mat");
				mc.Material = AssetManager::GetAsset<MaterialAsset>(matHandle);

				sphere.AddComponent<MaterialComponent>();
				auto& d = sphere.GetComponent<MaterialComponent>().Desc;

				MATERIALS mat = (MATERIALS)(i+1); // skip MATERIALS::None

				auto spec = GetMaterialType(mat);
				d.BaseColorFactor = spec.BaseColorFactor;
				d.MetallicFactor  = spec.MetallicFactor;
				d.RoughnessFactor = spec.RoughnessFactor;
				d.LightBoostFactor = spec.LightBoostFactor;

			}
		};

		int stride = 7;
		for (int i = 1; i < 6; i++)
			genMaterialSpheres((int)MATERIALS::Count, glm::vec3(-40.0f, -20.0f + (i * stride), 0.0f), stride, 3.0f, 'z');

	}
	{
		m_InfiniteGridShader = Shader::Create("infinite_grid.vert", "infinite_grid.frag");
		m_GridShader         = Shader::Create("grid.vert", "grid.frag");
		m_LightShader 		 = Shader::Create("light.vert", "light.frag");
		m_BaseShader  		 = Shader::Create("base.vert", "base.frag");
		m_PhongShader 		 = Shader::Create("phong.vert", "phong.frag");
		m_OutlineShader 	 = Shader::Create("outline.vert", "outline.frag");
		m_SkyboxShader  	 = Shader::Create("skybox.vert", "skybox.frag");
		m_PBRShader  	     = Shader::Create("pbr.vert", "pbr.frag");
		m_MaskShader		 = Shader::Create("mask.vert", "mask.frag");
		m_OutlinePostShader  = Shader::Create("outline_post.vert", "outline_post.frag");
		m_DepthOnlyShader    = Shader::Create("depth.vert", "depth.frag");

		// Create fallback textures
		unsigned char whitePixel[4] = {255, 255, 255, 255};
		unsigned char normalPixel[4] = {128, 128, 255, 255};

		// TODO: Refactor all Raw GL code
		glGenTextures(1, &m_WhiteTexture);
		glBindTexture(GL_TEXTURE_2D, m_WhiteTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

		glGenTextures(1, &m_FlatNormalTexture);
		glBindTexture(GL_TEXTURE_2D, m_FlatNormalTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, normalPixel);

		glBindTexture(GL_TEXTURE_2D, 0);

		m_CrosshairShader = Shader::Create("crosshair.vert", "crosshair.frag");
		m_CrosshairLayout.Push<float>(3);
		m_CrosshairVB = VertexBuffer(sizeof(PRIMITIVES::CrosshairVertices), PRIMITIVES::CrosshairVertices);
		m_CrosshairVA.AddBuffer(m_CrosshairVB, m_CrosshairLayout);
	}
	glGenVertexArrays(1, &m_InfiniteGridVAO);
	glGenVertexArrays(1, &m_GridVAO);
	{

		m_SkyboxLayout.Push<float>(3);
		m_SkyboxVB = VertexBuffer(sizeof(PRIMITIVES::SkyboxVerts), PRIMITIVES::SkyboxVerts);
		m_SkyboxVA.AddBuffer(m_SkyboxVB, m_SkyboxLayout);
		m_SkyboxVA.ResetAttribIndex();
		// ----------------- SKYBOX TEXTURE ----
		std::vector<std::string> faces = {
			"../assets/skyboxes/sky_13_2k/sky_13_cubemap_2k/px.png", // +X
			"../assets/skyboxes/sky_13_2k/sky_13_cubemap_2k/nx.png", // -X
			"../assets/skyboxes/sky_13_2k/sky_13_cubemap_2k/py.png",          // +Y
			"../assets/skyboxes/sky_13_2k/sky_13_cubemap_2k/ny.png",          // -Y
			"../assets/skyboxes/sky_13_2k/sky_13_cubemap_2k/pz.png",          // +Z
			"../assets/skyboxes/sky_13_2k/sky_13_cubemap_2k/nz.png"           // -Z
		};

		m_HDRSkyboxCubemap = LoadCubemap(faces);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_HDRSkyboxCubemap);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		m_MaxMip = (int)std::floor(std::log2(std::min(m_Width, m_Height)));
		m_SkyboxLoaded     = true;
	}


}


void Scene::Render(Renderer& renderer)
{
	renderer.EnableDepthTest(true);
	renderer.EnableDepthMask(true);

	Entity cam = GetPrimaryCameraEntity();
	if (!cam) return;
	auto& cc = cam.GetComponent<CameraComponent>();


	glm::vec2 viewport = m_Window.GetFramebufferViewport();

	int vpX = m_Window.GetRenderX();    // framebuffer pixels
	int vpY = m_Window.GetRenderY();
	int vpW = m_Window.GetRenderWidth();
	int vpH = m_Window.GetRenderHeight();

	glm::vec4 log = glm::vec4(vpX, vpY, vpW, vpH);

	renderer.BeginFrame(viewport.x, viewport.y, vpX, vpY, vpW, vpH);

	float sx = float(m_Window.GetFBW()) / float(m_Window.GetWindowWidth());
	float sy = float(m_Window.GetFBH()) / float(m_Window.GetWindowHeight());

	int vpW_fb = int(m_Window.GetRenderWidth()  * sx);
	int vpH_fb = int(m_Window.GetRenderHeight() * sy);

	cc.Camera.SetViewportSize(vpW_fb, vpH_fb);

	if (!m_OutlinesInitialized)
	{
		renderer.InitOutlines(vpW, vpH);
		m_OutlinesInitialized = true;
		m_LastOutlineW = vpW;
		m_LastOutlineH = vpH;
	}
	else if (vpW != m_LastOutlineW || vpH != m_LastOutlineH)
	{
		renderer.ResizeOutlines(vpW, vpH);
		m_LastOutlineW = vpW;
		m_LastOutlineH = vpH;
	}

	if (m_ShowSkybox) DrawSkybox(cc, renderer);

	glm::mat4 VP =
		cc.Camera.GetProjectionMatrix() *
		cc.Camera.GetViewMatrix();


	m_Lights.clear();
	auto lightView = m_Registry.view<TransformComponent, LightComponent>();
	for (auto e : lightView)
	{
		auto& tc = lightView.get<TransformComponent>(e);
		auto& lc = lightView.get<LightComponent>(e);

		SceneLight L;
		L.Position   = tc.Translation;
		L.Luminosity = lc.Luminosity;
		L.Color      = EntityUtils::ColorFromTemperature(lc.Temperature) * lc.TintColor;
		L.Radius     = tc.Scale.x;

		m_Lights.push_back(L);
	}

	auto view = m_Registry.view<TransformComponent, TagComponent, MeshComponent>();
	for (auto ent : view)
	{
		Entity e{ ent, &m_Registry };
		auto& tc   = m_Registry.get<TransformComponent>(e);
		auto& tag  = m_Registry.get<TagComponent>(e);
		auto& mc   = m_Registry.get<MeshComponent>(e);
		MaterialComponent* matc = m_Registry.try_get<MaterialComponent>(ent);

		if (!mc.MeshData) continue;

		GPUMesh& gpu = MeshRendererCache::GetOrCreate(*mc.MeshData);

		if (!gpu.BoundsReady)
		{
			glm::vec3 c;
			float r = MeshUtils::CalculateMeshRadius(*mc.MeshData, c);
			gpu.LocalBoundsCenter = c;
			gpu.LocalBoundsRadius = r;
			gpu.BoundsReady = true;
		}

		// CameraController* controller = m_CameraSystem->GetActiveController();
		// // If current controller is the third person controller
		// if (auto* tp = dynamic_cast<ThirdPersonCameraController*>(controller))
		// {
		// 	if (tp->GetEntity().GetUUID() == e.GetUUID())
		// 	{
		// 		tc.SetTranslation(tp->GetPosition());
		// 	}
		// }

		const auto& X = tc.GetCache();
		glm::mat4 basis = glm::toMat4(glm::quat(mc.BasisRotation));
		glm::mat4 model = X.Model * basis;

		glm::mat4 MVP = VP * model;

		bool hasMat = (matc != nullptr);


		if (e.HasComponent<ModelPartComponent>())
		{
			Entity root = EntityUtils::FindModelRootFromPart(*this, e);

			if (root.HasComponent<WireframeComponent>())
				renderer.DrawWireframe(m_OutlineShader, glm::vec3(0.0, 1.0, 1.0f), MVP, gpu.VA, gpu.IB);
		}
		else if (e.HasComponent<WireframeComponent>())
		{
			renderer.DrawWireframe(m_OutlineShader, glm::vec3(0.0, 1.0, 1.0f), MVP, gpu.VA, gpu.IB);
		}

		if (e.HasComponent<LightComponent>())
		{
			auto& lc = e.GetComponent<LightComponent>();
			glm::vec3 gizmoColor = EntityUtils::ColorFromTemperature(lc.Temperature) * lc.TintColor;

			m_LightShader->Bind();
			m_LightShader->SetMat4("u_MVP", MVP);
			m_LightShader->SetVec4("u_LightColor", glm::vec4(gizmoColor, 1.0f));

			if (!lc.HideLight || m_ShowLights)
				renderer.Draw(gpu.VA, gpu.IB);
			continue;
		}
		else if (!hasMat)
		{
			m_PhongShader->Bind();
			m_PhongShader->SetPhongUniforms(
				model,
				cc.Camera.GetProjectionMatrix(),
				m_Lights[0].Position,
				glm::vec4(m_Lights[0].Color, 1.0f),
				cc.Camera
			);

			m_PhongShader->SetBool("useNormalColors", mc.UseNormalColors);
			m_PhongShader->SetBool("useColor", true);
			m_PhongShader->SetVec4("u_Color", mc.BaseColor);

			renderer.Draw(gpu.VA, gpu.IB);
			continue;
		}
		else
		{
			Ref<Shader> shader = m_PBRShader;
			shader->Bind();
			shader->SetMat4("u_View", cc.Camera.GetViewMatrix());
			shader->SetMat4("u_Projection", cc.Camera.GetProjectionMatrix());
			shader->SetMat4("u_MVP", MVP);
			shader->SetMat4("u_Model", model);
			shader->SetVec3("u_CamPos", cc.Camera.GetPosition());

			int slot = 0;
			EntityUtils::BindMaterial(*this, *matc, shader, slot);

			shader->SetFloat("u_EnvIntensity", m_ShowSkybox ? 1.0f : 0.0f);
			shader->SetInt("u_EnvMap", slot);
			renderer.BindCubemap(m_HDRSkyboxCubemap, slot);

			slot++;

			shader->SetFloat("u_MaxEnvMip", (float)m_MaxMip);
			shader->SetFloat("u_Exposure", 1.0f);
			shader->SetFloat("u_EmissiveStrength", matc->Desc.EmissiveStrength);
			shader->SetFloat("u_LightBoost", matc->Desc.LightBoostFactor);
			shader->SetInt("u_LightCount", (int)m_Lights.size());

			for (int i = 0; i < m_Lights.size(); i++)
			{
				const SceneLight& L = m_Lights[i];
				std::string base = "u_Lights[" + std::to_string(i) + "].";

				shader->SetVec3(base + "Position",    L.Position);
				shader->SetFloat(base + "Luminosity", L.Luminosity);
				shader->SetVec3(base + "Color",       L.Color);
				shader->SetFloat(base + "Radius",     L.Radius);
				float tweakingScalar = 2.5f;
				float radiantIntensity = tweakingScalar * (L.Luminosity / 3.14159);
				shader->SetFloat(base + "Intensity", radiantIntensity);
			}

			renderer.Draw(gpu.VA, gpu.IB);
		}


	}

	if (m_ShowGrid)   DrawGrid(cc, renderer);
	DrawOutline(cc, renderer);
	// DrawOutlineDebugAll(cc, renderer);
	DrawScreenOverlays(cc, renderer);

}


void Scene::Update(float dt, Input& input)
{
	FrameContext ctx;
	auto cam = m_CameraSystem->GetPrimaryCamera();
	auto& cc = cam.GetComponent<CameraComponent>();


	m_RaycastSystem->Update(input, cc, m_Window, ctx);

	m_CameraSystem->Update(dt, input, ctx, this);

	m_SelectionSystem->Update(input, ctx);
	Entity selected = m_SelectionSystem->GetSelectedEntity();
	m_SelectedEntity = selected ? (entt::entity)selected : entt::null;

	m_DragSystem->Update(dt, input, cc, ctx);

	m_EditorCommandSystem->Update(input, ctx);

	m_UndoSystem->Update(input);


	SyncSubmeshes();
}


void Scene::DrawScreenOverlays(const CameraComponent& cc, Renderer& renderer)
{
	renderer.EnableDepthTest(false);
	renderer.EnableDepthMask(false);

	if (m_ShowCrosshair)
	{
		renderer.SetShader(m_CrosshairShader);
		m_CrosshairShader->SetVec3("u_Color", CROSSHAIR_COLOR);

		m_CrosshairVA.Bind();

		if (!m_CursorEnabled)
			renderer.DrawArrays(GL_LINES, 0, 4);
	}

	renderer.EnableDepthMask(true);
	renderer.EnableDepthTest(true);
}

void Scene::DrawGrid(const CameraComponent& cc, Renderer& renderer) const
{
    renderer.EnableBlending(true);
    renderer.SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderer.EnableDepthMask(false);

    renderer.SetShader(m_InfiniteGridShader);

    m_InfiniteGridShader->SetFloat("GridHeight", 0.0f);
    m_InfiniteGridShader->SetMat4("Projection", cc.Camera.GetProjectionMatrix());
    m_InfiniteGridShader->SetMat4("View", cc.Camera.GetViewMatrix());
    m_InfiniteGridShader->SetVec3("CameraWorldPos", cc.Camera.GetPosition());
    m_InfiniteGridShader->SetBool("ShowAxes", m_ShowAxes);

	renderer.BindVertexArray(m_InfiniteGridVAO);

    renderer.DrawArrays(GL_TRIANGLES, 0, 6);

    renderer.EnableDepthMask(true);
    renderer.EnableBlending(false);
}


void Scene::DrawSkybox(const CameraComponent& cc, Renderer& renderer) const
{
	if (!m_SkyboxLoaded) return;

	renderer.SetDepthFunc(GL_LEQUAL);
	renderer.EnableDepthMask(false);

	renderer.SetShader(m_SkyboxShader);

	glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(cc.Camera.GetViewMatrix()));

	m_SkyboxShader->SetMat4("u_Projection", cc.Camera.GetProjectionMatrix());
	m_SkyboxShader->SetMat4("u_View", viewNoTranslate);

	renderer.BindCubemap(m_HDRSkyboxCubemap, 0);
	m_SkyboxShader->SetInt("u_Skybox", 0);

	m_SkyboxShader->SetInt("u_Skybox", 0);
	m_SkyboxShader->SetFloat("u_Exposure", 1.5f);

	m_SkyboxVA.Bind();
	renderer.DrawArrays(GL_TRIANGLES, 0, 36);

	renderer.EnableDepthMask(true);
	renderer.SetDepthFunc(GL_LESS);
}

void Scene::DrawLocalGrid(const CameraComponent& cc,
						  Renderer& renderer,
						  const glm::vec3& origin,
						  float height,
						  float size)
{
	renderer.EnableBlending(true);
	renderer.SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	renderer.EnableDepthMask(false);

	glm::mat4 VP =
		cc.Camera.GetProjectionMatrix() *
		cc.Camera.GetViewMatrix();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), origin);

	renderer.SetShader(m_GridShader);

	m_GridShader->SetMat4("Projection", cc.Camera.GetProjectionMatrix());
	m_GridShader->SetMat4("View",       cc.Camera.GetViewMatrix());
	m_GridShader->SetMat4("u_Model",    model);
	m_GridShader->SetVec3("CameraWorldPos", origin);
	m_GridShader->SetFloat("GridHeight",    height);
	m_GridShader->SetFloat("gGridSize",     size);
	m_GridShader->SetFloat("gGridCellSize", 1.0f);

	renderer.BindVertexArray(m_GridVAO);
	renderer.DrawArrays(GL_TRIANGLES, 0, 6);

	renderer.EnableDepthMask(true);
	renderer.EnableBlending(false);
}



void Scene::DrawYAxis(const CameraComponent& cc, Renderer& renderer, Entity selected)
{
	Entity yAxis = GetEntityByName("Y-Axis");


	const glm::vec3 center = selected.GetComponent<TransformComponent>().Translation;
	auto& axisTC = yAxis.GetComponent<TransformComponent>();
	axisTC.Translation = center;
	axisTC.MarkDirty();

	glm::mat4 VP =
		cc.Camera.GetProjectionMatrix() *
		cc.Camera.GetViewMatrix();

	const auto& X = axisTC.GetCache();
	const glm::mat4& model = X.Model;
	glm::mat4 MVP = VP * model;

	auto& axisMC = yAxis.GetComponent<MeshComponent>();


	GPUMesh& gpu = MeshRendererCache::GetOrCreate(*axisMC.MeshData);

	m_BaseShader->Bind();
	renderer.SetShader(m_BaseShader);
	m_BaseShader->SetMat4("u_MVP", MVP);
	m_BaseShader->SetMat4("u_Model", model);
	renderer.DrawLines(gpu.VA, gpu.IB);
}

void Scene::DrawOutlineDebugAll(const CameraComponent& cc, Renderer& renderer)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer.GetSelectionFBO());
	glBlitFramebuffer(
		0, 0, m_LastOutlineW, m_LastOutlineH,
		0, 0, m_LastOutlineW, m_LastOutlineH,
		GL_DEPTH_BUFFER_BIT,
		GL_NEAREST
	);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	renderer.BeginSelectionMask();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	auto view = m_Registry.view<TransformComponent, MeshComponent>();
	for (auto raw : view)
	{
		Entity e{ raw, &m_Registry };

		auto& tc = m_Registry.get<TransformComponent>(e);
		auto& mc = m_Registry.get<MeshComponent>(e);
		if (!mc.MeshData)
			continue;

		glm::mat4 VP = cc.Camera.GetProjectionMatrix() *
					   cc.Camera.GetViewMatrix();
		GPUMesh& gpu = MeshRendererCache::GetOrCreate(*mc.MeshData);
		glm::mat4 basis = glm::toMat4(glm::quat(mc.BasisRotation));
		glm::mat4 MVP = VP * tc.GetCache().Model * basis;

		renderer.DrawSelectionMask(m_MaskShader, MVP, gpu.VA, gpu.IB);
	}

	renderer.EndSelectionMask();
	renderer.CompositeOutlines(
		m_OutlinePostShader,
		glm::vec3(0.980392f, 0.647059f, 0.027451f),
		4
	);
}

void Scene::DrawOutline(const CameraComponent& cc, Renderer& renderer)
{
	Entity selectedRoot{};

	auto selectedView = m_Registry.view<SelectedComponent>();
	for (auto rawSel : selectedView)
	{
		selectedRoot = Entity{ rawSel, &m_Registry };
		break;
	}

	if (selectedRoot)
	{
		if (m_DragSystem->DragAffectsVertical)
			DrawYAxis(cc, renderer, selectedRoot);

		if (m_DragSystem->DragAffectsXZ)
			DrawLocalGrid(cc,
						  renderer,
						  m_DragSystem->GetLocalGridOrigin(),
						  m_DragSystem->GetLocalGridHeight(),
						  m_DragSystem->GetLocalGridSize());
	}


	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer.GetSelectionFBO());
	glBlitFramebuffer(
		0, 0, m_LastOutlineW, m_LastOutlineH,
		0, 0, m_LastOutlineW, m_LastOutlineH,
		GL_DEPTH_BUFFER_BIT,
		GL_NEAREST
	);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	renderer.BeginSelectionMask();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	auto view = m_Registry.view<TransformComponent, TagComponent, MeshComponent>();
	for (auto raw : view)
	{
		Entity e{ raw, &m_Registry };

		bool isSelected = e.HasComponent<SelectedComponent>();

		if (!isSelected && e.HasComponent<ModelPartComponent>())
		{
			auto& part = e.GetComponent<ModelPartComponent>();
			Entity root = GetEntityByID(part.RootID);
			if (root && root.HasComponent<SelectedComponent>())
				isSelected = true;
		}

		if (!isSelected)
			continue;

		auto& tc = m_Registry.get<TransformComponent>(e);
		auto& mc = m_Registry.get<MeshComponent>(e);

		if (!mc.MeshData)
			continue;

		glm::mat4 VP = cc.Camera.GetProjectionMatrix() *
					   cc.Camera.GetViewMatrix();
		GPUMesh& gpu = MeshRendererCache::GetOrCreate(*mc.MeshData);
		glm::mat4 basis = glm::toMat4(glm::quat(mc.BasisRotation));
		glm::mat4 MVP = VP * tc.GetCache().Model * basis;

		renderer.DrawSelectionMask(m_MaskShader, MVP, gpu.VA, gpu.IB);
	}

	renderer.EndSelectionMask();
	renderer.CompositeOutlines(
		m_OutlinePostShader,
		glm::vec3(0.980392f, 0.647059f, 0.027451f),
		4
	);
}



Ref<Shader> Scene::GetShader(UUID id)
{
	static std::unordered_map<UUID, Ref<Shader>> cache;

	if (cache.contains(id))
		return cache[id];

	auto shaderAsset = AssetManager::GetAsset<ShaderAsset>(id);

	Ref<Shader> shader = Shader::Create(
		shaderAsset->VertexSource,
		shaderAsset->FragmentSource
	);

	cache[id] = shader;
	return shader;
}

void Scene::SyncSubmeshes()
{
	for (auto e : m_Registry.view<ModelRootComponent, TransformComponent>())
	{
		auto root = Entity{e, &m_Registry};
		auto& rootTC = root.GetComponent<TransformComponent>();
		auto& modelRoot = root.GetComponent<ModelRootComponent>();

		// Only sync if root transform changed
		if (rootTC.Translation != modelRoot.LastTranslation ||
			rootTC.Rotation != modelRoot.LastRotation ||
			rootTC.Scale != modelRoot.LastScale)
		{
			for (UUID id : modelRoot.Parts)
			{
				Entity child = GetEntityByID(id);
				if (!child) continue;
				auto& tc = child.GetComponent<TransformComponent>();
				tc.Translation = rootTC.Translation;
				tc.Rotation = rootTC.Rotation;
				tc.Scale = rootTC.Scale;
				tc.MarkDirty();
			}

			modelRoot.LastTranslation = rootTC.Translation;
			modelRoot.LastRotation = rootTC.Rotation;
			modelRoot.LastScale = rootTC.Scale;
		}
	}
}


void Scene::SetSelectedEntity(entt::entity e)
{
	m_SelectedEntity = e;

	if (e == entt::null || !m_Registry.valid(e) || !m_Registry.all_of<IDComponent>(e))
	{
		m_SelectedUUID = {};
		return;
	}

	m_SelectedUUID = m_Registry.get<IDComponent>(e).ID;
}


Entity Scene::RestoreEntityFromSnapshot(const EntitySnapshot& s)
{
	{
		Entity existing = GetEntityByID(s.id);
		if (existing) DeleteEntity(existing);
	}

	Entity e = CreateEntityForRestore(s.id, s.tag);

	e.GetComponent<TagComponent>().Tag = s.tag;

	if (s.hasTransform)
	{
		e.GetComponent<TransformComponent>() = s.transform;
		e.GetComponent<TransformComponent>().MarkDirty();
	}

	if (s.hasMesh)
	{
		if (!e.HasComponent<MeshComponent>())
			e.AddComponent<MeshComponent>(s.mesh);
		else
			e.GetComponent<MeshComponent>() = s.mesh;
	}

	if (s.hasLight)
	{
		if (!e.HasComponent<LightComponent>())
			e.AddComponent<LightComponent>(s.light);
		else
			e.GetComponent<LightComponent>() = s.light;
	}

	if (s.hasMaterial)
	{
		if (!e.HasComponent<MaterialComponent>())
			e.AddComponent<MaterialComponent>(s.material);
		else
			e.GetComponent<MaterialComponent>() = s.material;
	}

	if (s.hasModelRoot)
	{
		if (!e.HasComponent<ModelRootComponent>())
			e.AddComponent<ModelRootComponent>();

		auto& mr = e.GetComponent<ModelRootComponent>();
		mr.Parts.clear();
		mr.Parts.reserve(s.parts.size());

		for (const EntitySnapshot& ps : s.parts)
		{
			Entity part = RestoreEntityFromSnapshot(ps);
			if (part)
				mr.Parts.push_back(part.GetUUID());
		}

		mr.LastTranslation = e.GetComponent<TransformComponent>().Translation;
		mr.LastRotation    = e.GetComponent<TransformComponent>().Rotation;
		mr.LastScale       = e.GetComponent<TransformComponent>().Scale;
	}

	if (e.HasComponent<SelectedComponent>())
		e.RemoveComponent<SelectedComponent>();

	return e;
}

EntitySnapshot Scene::SnapshotEntity(Entity e)
{
	EntitySnapshot s;
	if (!e) return s;
	s.id  = e.GetUUID();
	s.tag = e.GetTag();

	if (e.HasComponent<TransformComponent>())
	{
		s.hasTransform = true;
		s.transform = e.GetComponent<TransformComponent>();
	}

	if (e.HasComponent<MeshComponent>())
	{
		s.hasMesh = true;
		s.mesh = e.GetComponent<MeshComponent>();
	}

	if (e.HasComponent<LightComponent>())
	{
		s.hasLight = true;
		s.light = e.GetComponent<LightComponent>();
	}

	if (e.HasComponent<MaterialComponent>())
	{
		s.hasMaterial = true;
		s.material = e.GetComponent<MaterialComponent>();
	}

	if (e.HasComponent<ModelRootComponent>())
	{
		s.hasModelRoot = true;

		const auto& mr = e.GetComponent<ModelRootComponent>();
		s.parts.clear();
		s.parts.reserve(mr.Parts.size());

		for (UUID pid : mr.Parts)
		{
			Entity part = GetEntityByID(pid);
			if (!part) continue;
			s.parts.push_back(SnapshotEntity(part));
		}
	}

	return s;
}


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
			return Entity{e, &m_Registry};
	}

	return {};
}

Entity Scene::GetEntityByName(const std::string& name)
{
	auto it = m_EntityNameMap.find(name);
	if (it != m_EntityNameMap.end())
		return Entity{ it->second, &m_Registry };

	return {};
}

Entity Scene::GetEntityByID(UUID id)
{
	auto it = m_EntityMap.find(id);
	if (it != m_EntityMap.end())
		return Entity{ it->second, &m_Registry };

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


Entity Scene::CreateEntity(UUID uuid, const std::string& name)
{
	Entity entity = { m_Registry.create(), &m_Registry };
	entity.AddComponent<IDComponent>(uuid);
	entity.AddComponent<TransformComponent>();

	auto& tag = entity.AddComponent<TagComponent>();
	tag.Tag = name.empty() ? "Entity" : name;

	m_EntityMap[uuid] = entity;

	if (!name.empty())
		m_EntityNameMap[tag.Tag] = entity;

	return entity;
}

Entity Scene::CreateEntityForRestore(UUID uuid, const std::string& name)
{
	Entity entity = { m_Registry.create(), &m_Registry };

	entity.AddComponent<IDComponent>(uuid);
	entity.AddComponent<TransformComponent>();

	auto& tag = entity.AddComponent<TagComponent>();
	tag.Tag = name;

	m_EntityMap[uuid] = entity;

	if (!name.empty())
		m_EntityNameMap[name] = entity;

	return entity;
}

Entity Scene::CreateEntityFromModel(const std::filesystem::path& path,
								   const std::string& rootName)
{
	// Import the .glb (auto-generates .mesh, .mat, .model)
	ModelImporter::Import(path);

	// Form the expected .model path
	std::filesystem::path modelPath =
		path.parent_path() / (path.stem().string() + ".model");

	// Normalize the path to match registry format
	std::string rel = modelPath.lexically_normal().string();

	const std::string prefix = "../assets/";
	if (rel.starts_with(prefix))
		rel = rel.substr(prefix.size());

	// Look up the asset in the registry
	AssetHandle handle = AssetManager::GetHandleForPath(rel);
	if (!handle)
		return {}; // Not found

	Ref<ModelAsset> model = AssetManager::GetAsset<ModelAsset>(handle);

	// Create entities for each submesh
	Entity root = CreateEntity(UUID(), rootName);
	auto& rootTC = root.GetComponent<TransformComponent>();

	auto& rootModel = root.AddComponent<ModelRootComponent>();
	rootModel.RootID = root.GetUUID();

	for (auto& sm : model->Submeshes)
	{
		UUID partID = UUID();
		Entity part = CreateEntity(partID, rootName + "_Part");
		rootModel.Parts.push_back(partID);

		auto& partTC = part.GetComponent<TransformComponent>();
		partTC.Translation = rootTC.Translation;
		partTC.Rotation    = rootTC.Rotation;
		partTC.Scale       = rootTC.Scale;
		partTC.MarkDirty();

		auto& mc = part.AddComponent<MeshComponent>();
		mc.MeshData = sm.Mesh->MeshData;
		mc.Material = sm.Material;

		auto& matc = part.AddComponent<MaterialComponent>();
		matc.BaseMaterial = sm.Material ? sm.Material->Handle : UUID(0);
		matc.Desc         = sm.Material ? sm.Material->Desc : MaterialDesc{};

		auto& partComp = part.AddComponent<ModelPartComponent>();
		partComp.RootID = root.GetUUID();

		m_EntityNameMap.erase(part.GetName());
	}


	return root;
}

void Scene::DeleteEntity(Entity entity)
{
	if (!entity)
		return;

	// If this is a model root, delete all parts first
	if (entity.HasComponent<ModelRootComponent>())
	{
		auto& root = entity.GetComponent<ModelRootComponent>();

		for (UUID id : root.Parts)
		{
			Entity part = GetEntityByID(id);
			if (part)
			{
				m_EntityMap.erase(part.GetUUID());
				m_EntityNameMap.erase(part.GetName());
				m_Registry.destroy(part);
			}
		}
	}

	if (entity.HasComponent<ModelRootComponent>())
	{
		auto& root = entity.GetComponent<ModelRootComponent>();
		for (UUID id : root.Parts)
		{
			Entity part = GetEntityByID(id);
			if (part)
			{
				m_EntityMap.erase(part.GetUUID());
				m_EntityNameMap.erase(part.GetName());
				m_Registry.destroy((entt::entity)part);
			}
		}
	}

	m_EntityMap.erase(entity.GetUUID());
	m_EntityNameMap.erase(entity.GetName());
	m_Registry.destroy((entt::entity)entity);
}

static std::string StripCopySuffix(const std::string& name)
{
	size_t pos = name.find(" [Copy");
	if (pos != std::string::npos)
		return name.substr(0, pos);

	return name;
}

std::string Scene::GenerateCopyName(const std::string& base)
{
	int index = 1;
	std::string candidate;

	do
	{
		candidate = base + " [Copy " + std::to_string(index) + "]";
		index++;
	}
	while (m_EntityNameMap.contains(candidate));

	return candidate;
}

Entity Scene::DuplicateEntity(Entity src)
{
    if (!src) return {};

    if (src.HasComponent<ModelPartComponent>())
    {
        auto& part = src.GetComponent<ModelPartComponent>();
        src = GetEntityByID(part.RootID);
        if (!src) return {};
    }

    std::string srcTag = src.GetTag();

    std::string base = StripCopySuffix(srcTag);

    const std::string partSuffix = "_Part";
    if (base.size() > partSuffix.size() &&
        base.rfind(partSuffix) == base.size() - partSuffix.size())
    {
        base.erase(base.size() - partSuffix.size());
    }

    std::string newName = GenerateCopyName(base);
    Entity dst = CreateEntity(UUID(), newName);

    auto copyOrAssign = [&](auto tagType)
    {
        using T = decltype(tagType);
        if (!src.HasComponent<T>()) return;

        if (dst.HasComponent<T>())
            dst.GetComponent<T>() = src.GetComponent<T>();
        else
            dst.AddComponent<T>(src.GetComponent<T>());
    };

    if (src.HasComponent<TransformComponent>())
    {
        dst.GetComponent<TransformComponent>() = src.GetComponent<TransformComponent>();
        dst.GetComponent<TransformComponent>().MarkDirty();
    }

    constexpr float DUPLICATE_OFFSET = 5.0f;

    auto& tc = dst.GetComponent<TransformComponent>();
    tc.Translation += glm::vec3(DUPLICATE_OFFSET, 0.0f, 0.0f);
    tc.MarkDirty();

    glm::vec3 delta =
        dst.GetComponent<TransformComponent>().Translation -
        src.GetComponent<TransformComponent>().Translation;

    copyOrAssign(MeshComponent{});
    copyOrAssign(MaterialComponent{});
    copyOrAssign(LightComponent{});

    if (dst.HasComponent<SelectedComponent>())
        dst.RemoveComponent<SelectedComponent>();

    if (src.HasComponent<ModelRootComponent>())
    {
        const auto& srcRoot = src.GetComponent<ModelRootComponent>();

        if (!dst.HasComponent<ModelRootComponent>())
            dst.AddComponent<ModelRootComponent>();

        auto& dstRoot = dst.GetComponent<ModelRootComponent>();
        dstRoot.Parts.clear();
        dstRoot.RootID = dst.GetUUID();

        for (UUID partID : srcRoot.Parts)
        {
            Entity srcPart = GetEntityByID(partID);
            if (!srcPart) continue;

            Entity dstPart = CreateEntity(UUID(), newName + "_Part");

            if (srcPart.HasComponent<TransformComponent>())
            {
                dstPart.GetComponent<TransformComponent>() =
                    srcPart.GetComponent<TransformComponent>();

                dstPart.GetComponent<TransformComponent>().Translation += delta;
                dstPart.GetComponent<TransformComponent>().MarkDirty();
            }

            if (srcPart.HasComponent<MeshComponent>())
                dstPart.AddComponent<MeshComponent>(
                    srcPart.GetComponent<MeshComponent>());

            if (srcPart.HasComponent<MaterialComponent>())
                dstPart.AddComponent<MaterialComponent>(
                    srcPart.GetComponent<MaterialComponent>());

            if (dstPart.HasComponent<SelectedComponent>())
                dstPart.RemoveComponent<SelectedComponent>();

            auto& partComp = dstPart.AddComponent<ModelPartComponent>();
            partComp.RootID = dst.GetUUID();

            dstRoot.Parts.push_back(dstPart.GetUUID());
        }

        dstRoot.LastTranslation =
            dst.GetComponent<TransformComponent>().Translation;
        dstRoot.LastRotation =
            dst.GetComponent<TransformComponent>().Rotation;
        dstRoot.LastScale =
            dst.GetComponent<TransformComponent>().Scale;
    }

    return dst;
}

template<typename T>
void Scene::OnComponentAdded(Entity, T&) {}

template<>
void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
{
	if (component.Primary)
		SetPrimaryCamera(entity);
}

