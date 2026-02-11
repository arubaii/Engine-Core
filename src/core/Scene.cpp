#include "Scene.h"

#include "Application.h"
#include "asset_core/AssetManager.h"
#include "asset_io/ModelImporter.h"
#include "scene_core/Entity.h"
#include "utils/Log.h"
#include "io/MouseCodes.h"
#include "asset_io/LoadCursor.h"
#include "renderer_core/GLHDRTexture2D.h"


static void DeleteSnapshotRecursive(Scene* scene, const EntitySnapshot& s)
{
	// Delete children first
	for (const EntitySnapshot& ps : s.parts)
		DeleteSnapshotRecursive(scene, ps);

	// Then delete this entity
	Entity e = scene->GetEntityByID(s.id);
	if (e) scene->DeleteEntity(e);
}


static GLuint s_CubeVAO = 0;
static GLuint s_CubeVBO = 0;

static void RenderCube()
{
    if (s_CubeVAO == 0)
    {
        glGenVertexArrays(1, &s_CubeVAO);
        glGenBuffers(1, &s_CubeVBO);

        glBindVertexArray(s_CubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_CubeVBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(PRIMITIVES::SkyboxVerts), PRIMITIVES::SkyboxVerts, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(s_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

static GLuint s_QuadVAO = 0;
static GLuint s_QuadVBO = 0;

static void RenderQuad()
{
    if (s_QuadVAO == 0)
    {
        // pos (2) + uv (2)
        float quadVerts[] = {
            //  x     y     u     v
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,

            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 1.0f
        };

        glGenVertexArrays(1, &s_QuadVAO);
        glGenBuffers(1, &s_QuadVBO);

        glBindVertexArray(s_QuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_QuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(s_QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

static GLuint LoadCubemap(const std::vector<std::string>& faces)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	int w, h, n;
	stbi_set_flip_vertically_on_load(false);

	for (int i = 0; i < 6; i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &n, 3);
		if (!data)
		{
			std::cerr << "Failed to load cubemap face: " << faces[i] << std::endl;
			continue;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					 0, GL_RGB8, w, h, 0,
					 GL_RGB, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texID;
}

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
	{
		m_PointHandCursor  = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		m_ArrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_GrabHandCursor = LoadCursor("../assets/cursors/handgrabbing.png");		 // macos
		m_OpenHandCursor = LoadCursor("../assets/cursors/handopen.png");			 // macos
		m_ResizeUpDownCursor = LoadCursor("../assets/cursors/resizeupdown.png");	 // macos
		m_MoveCursor = LoadCursor("../assets/cursors/move.png");					 // macos

	}

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
		yAxisComponent.MeshData->Indices = PRIMITIVES::yAxisIndices;

		auto& ytc = yAxis.GetComponent<TransformComponent>();
		ytc.Translation = {0.0f, -2.5f, 0.0f};
		ytc.MarkDirty();
	}

	{

		auto genMaterialSpheres = [this](int N, glm::vec3 firstSpherePos, int stride, float scale, const char* dir)
		{

			for (int i = 0; i < N; i++)
			{

				Entity sphere = CreateEntity(UUID(), "Sphere " + std::to_string(i));

				auto&tc = sphere.GetComponent<TransformComponent>();

				if (dir == "x")
					tc.Translation = firstSpherePos + glm::vec3(float(stride * i), 0.0f, 0.0f);
				else if (dir == "y")
					tc.Translation = firstSpherePos + glm::vec3(0.0f, float(stride * i), 0.0f);
				else if (dir == "z")
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
			genMaterialSpheres((int)MATERIALS::Count, glm::vec3(-40.0f, -20.0f + (i * stride), 0.0f), stride, 3.0f, "z");

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
	{
		CreateEntityFromModel("../assets/models/suzanne/suzanne.glb", "Suzanne");
		auto& tcSuz = GetEntityByName("Suzanne").GetComponent<TransformComponent>();
		tcSuz.Scale *= glm::vec3(10.0f);
		tcSuz.MarkDirty();

		CreateEntityFromModel("../assets/models/stanford_bunny/scene.gltf", "Stanford Bunny");
		auto& tcBunny = GetEntityByName("Stanford Bunny").GetComponent<TransformComponent>();
		tcBunny.Scale *= glm::vec3(1.0f);
		tcBunny.Translation += glm::vec3(25.0f, 0.0f, 0.0f);
		tcBunny.Rotation.x = glm::radians(270.0f);
		tcBunny.MarkDirty();


		CreateEntityFromModel("../assets/models/space_helmet/scene.gltf", "Space Helmet");
		auto& tcSH = GetEntityByName("Space Helmet").GetComponent<TransformComponent>();
		tcSH.Scale *= glm::vec3(5.0f);
		tcSH.Translation += glm::vec3(45.0f, 0.0f, 0.0f);
		tcSH.Rotation.x = glm::radians(90.0f);
		tcSH.MarkDirty();
		{
			Entity bunny = GetEntityByName("Stanford Bunny");
			AssetHandle h = AssetManager::GetHandleForPath("materials/test_pbr.mat");
			Ref<MaterialAsset> mat = AssetManager::GetAsset<MaterialAsset>(h);

			SetModelMaterial(bunny, h);
		}
		{
			Entity suz = GetEntityByName("Suzanne");
			AssetHandle h = AssetManager::GetHandleForPath("materials/test_pbr.mat");
			Ref<MaterialAsset> mat = AssetManager::GetAsset<MaterialAsset>(h);

			SetModelMaterial(suz, h);
		}


	}


}

void Scene::Render(Renderer& renderer)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	Entity cam = GetPrimaryCameraEntity();
	if (!cam) return;
	auto& cc = cam.GetComponent<CameraComponent>();

	if (m_ShowSkybox) DrawSkybox(cc);

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
		L.Color      = ColorFromTemperature(lc.Temperature) * lc.TintColor;
		L.Radius     = tc.Scale.x;

		m_Lights.push_back(L);
	}

	auto view = m_Registry.view<TransformComponent, TagComponent, MeshComponent>();
	for (auto raw : view)
	{
		Entity e{ raw, this };
		auto& tc   = m_Registry.get<TransformComponent>(e);
		auto& tag  = m_Registry.get<TagComponent>(e);
		auto& mc   = m_Registry.get<MeshComponent>(e);
		MaterialComponent* matc = m_Registry.try_get<MaterialComponent>(raw);

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

		const auto& X = tc.GetCache();
		const glm::mat4& model = X.Model;
		glm::mat4 MVP = VP * model;


		bool hasMat = (matc != nullptr);


		if (e.HasComponent<SelectedComponent>())
		{
			if (m_DragAffectsXZ && m_IsDragging)
				DrawLocalGrid(cc);

			if (m_DragAffectsVertical && m_IsDragging)
				DrawYAxis(cc, renderer, e);


			renderer.DrawWireframe(m_OutlineShader, MVP, gpu.VA, gpu.IB);
		}

		if (e.HasComponent<LightComponent>())
		{

			auto& lc = e.GetComponent<LightComponent>();

			glm::vec3 gizmoColor = ColorFromTemperature(lc.Temperature) * lc.TintColor;

			m_LightShader->Bind();
			m_LightShader->SetMat4("u_MVP", MVP);
			m_LightShader->SetVec4("u_LightColor", glm::vec4(gizmoColor, 1.0f));

			if (!lc.HideLight || m_ShowLights)
				renderer.Draw(gpu.VA, gpu.IB);
			continue;
		}
		else if (!hasMat)
		{
			// Fallback to phong
			m_PhongShader->Bind();
			m_PhongShader->SetPhongUniforms(
				model,
				cc.Camera.GetProjectionMatrix(),
				m_Lights[0].Position,     // still works if no lights
				glm::vec4(m_Lights[0].Color, 1.0f),
				cc.Camera
			);

			m_PhongShader->SetBool("useNormalColors", mc.UseNormalColors);
			m_PhongShader->SetBool("useColor", true);
			m_PhongShader->SetVec4("u_Color", mc.BaseColor);

			renderer.Draw(gpu.VA, gpu.IB);
			continue;
		}
		// PBR Render path
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

			BindMaterial(*matc, shader, slot);



			shader->SetFloat("u_EnvIntensity", m_ShowSkybox ? 1.0f : 0.0f);
			shader->SetInt("u_EnvMap", slot);
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_HDRSkyboxCubemap);
			slot++;

			shader->SetFloat("u_MaxEnvMip", (float)m_MaxMip);

			shader->SetFloat("u_Exposure", 1.0f);
			shader->SetFloat("u_EmissiveStrength", matc->Desc.EmissiveStrength);
			shader->SetFloat("u_LightBoost", matc->Desc.LightBoostFactor);


			// Upload lights
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

	if (m_ShowGrid)   DrawGrid(cc);
	DrawScreenOverlays(cc, renderer);
}

void Scene::Update(float dt, Input& input)
{

	{
		m_CursorEnabled = input.IsCursorEnabled();
	}

	// ==================== Camera ========================
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
	bool hasHit = false;

	// Only re-raycast if mouse moved OR camera changed
	// If cursor disabled, use center, so mouse moved won’t matter
	glm::vec2 currMouse = input.IsCursorEnabled() ? input.GetMousePos() : center;

	bool mouseChanged =
		glm::length(currMouse - m_LastRayMouse) > 0.25f; // tweak threshold

	bool camChanged =
		glm::length(cc.Camera.GetPosition() - m_LastCamPos) > 1e-4f ||
		std::abs(cc.Camera.GetYaw()   - m_LastCamYaw)   > 1e-5f ||
		std::abs(cc.Camera.GetPitch() - m_LastCamPitch) > 1e-5f;

	if (mouseChanged || camChanged)
	{
		m_LastRayMouse = currMouse;
		m_LastCamPos   = cc.Camera.GetPosition();
		m_LastCamYaw   = cc.Camera.GetYaw();
		m_LastCamPitch = cc.Camera.GetPitch();

		m_LastRayHit = RayHit{};
		m_LastHasHit = Raycast(ray, m_LastRayHit);
	}

	hit = m_LastRayHit;
	hasHit = m_LastHasHit && hit.entity != entt::null && !input.IsInUI();

	#ifdef __APPLE__
		bool ctrlOrCmd =
			input.IsKeyPressed(Key::LeftCmd)   || input.IsKeyPressed(Key::RightCmd);
	#else
		bool ctrlOrCmd =
			input.IsKeyPressed(Key::LeftCtrl)  || input.IsKeyPressed(Key::RightCtrl);
	#endif

	bool shift =
		input.IsKeyPressed(Key::LeftShift) || input.IsKeyPressed(Key::RightShift);

	bool zOnce = input.IsKeyPressedOnce(Key::Z);

	if (ctrlOrCmd && zOnce)
	{
		if (shift) Redo();
		else       Undo();
	}

	// Duplicate entity
	if (hasHit &&
		input.IsKeyPressed(Key::LeftShift) &&
		input.IsKeyPressedOnce(Key::K))
	{
		Entity src{ hit.entity, this };
		if (src)
		{
			Entity copy = DuplicateEntity(src);

			// Keep it from perfectly overlapping
			if (copy.HasComponent<TransformComponent>())
			{
				auto& tc = copy.GetComponent<TransformComponent>();
				tc.Translation += glm::vec3(2.0f, 0.0f, 0.0f);
				tc.MarkDirty();
			}

			// Record creation for undo/redo
			CreateAction act;
			act.snapshot = SnapshotEntity(copy);
			PushAction(act);

			auto selView = m_Registry.view<SelectedComponent>();
			for (auto eRaw : selView)
				Entity{ eRaw, this }.RemoveComponent<SelectedComponent>();

			copy.AddComponent<SelectedComponent>();
			SetSelectedEntity(copy);
		}
	}


	// Delete entity
	if (hasHit &&
		input.IsKeyPressed(Key::LeftShift) &&
		input.IsKeyPressedOnce(Key::L))
	{
		Entity victim{ hit.entity, this };
		if (victim)
		{
			// If this is a model part, delete the root instead
			Entity root = victim.HasComponent<ModelRootComponent>() ? victim : FindModelRootFromPart(victim);
			if (!root) root = victim;

			DeleteAction act;
			act.snapshot = SnapshotEntity(root);
			PushAction(act);

			if (root.HasComponent<SelectedComponent>())
				SetSelectedEntity(entt::null);

			DeleteEntity(root);
		}
	}

	if (input.IsKeyPressedOnce(Key::P) || input.IsMousePressedOnce(Mouse::Middle))
	{
		if (m_ActiveController == ORBIT_CONTROLLER_INDEX)
			m_ActiveController = FREE_CONTROLLER_INDEX;

		else if (hasHit)
		{
			CROSSHAIR_COLOR = glm::vec3(1.0f, 0.0f, 0.0f);
			Entity entity{ hit.entity, this };

			// Calculate appropriate orbit distance based on mesh size
			float radius = CalculateEntityRadius(entity);

			float viewportCoverage = 0.5f;
			float fovRadians = glm::radians(m_CameraProps.Fov * 0.5f);
			float orbitDistance = radius / (glm::tan(fovRadians) * viewportCoverage);
			orbitDistance = glm::clamp(orbitDistance, 0.01f, 50000.0f);


			m_ActiveController = ORBIT_CONTROLLER_INDEX;
			m_CameraControllers[m_ActiveController]->OnSelect(entity.GetPosition(), orbitDistance);
		}
	}
	// =================================================================================
	// =================================================================================

	SyncSubmeshes();

	// =================================================================================
	// =================================================================================

	// ============== Ray casting Updates ============
	if (hasHit && !m_WasHovering)
	{
		m_WasHovering = true;
		glfwSetCursor(m_Window.GetGLFWwindow(), m_OpenHandCursor);
	}
	else if (!hasHit && m_WasHovering && !m_IsDragging)
	{
		m_WasHovering = false;
		if (m_ActiveController == ORBIT_CONTROLLER_INDEX)
			glfwSetCursor(m_Window.GetGLFWwindow(), m_PointHandCursor);
		else
			glfwSetCursor(m_Window.GetGLFWwindow(), m_ArrowCursor);
	}

	CROSSHAIR_COLOR = hasHit ? glm::vec3(1,0,0) : glm::vec3(1);


	// Geometry Drag & Drop
	if ( !m_IsDragging &&
	     input.IsKeyPressed(Key::LeftShift) && input.IsMousePressedOnce(Mouse::Left) &&
		 m_ActiveController == FREE_CONTROLLER_INDEX &&
		 hasHit )
	{


		m_DraggedEntity = hit.entity;

		Entity dragged{ m_DraggedEntity, this };
		if (dragged.HasComponent<SelectedComponent>())
		{
			glfwSetCursor(m_Window.GetGLFWwindow(), m_GrabHandCursor);

			m_IsDragging = true;
			m_DragOffset = dragged.GetPosition() - hit.position;

		}
	}

	// Geometry select
	else if (input.IsMousePressedOnce(Mouse::Left) && hasHit)
	{

		SetSelectedEntity(hit.entity);
		Entity selected{ hit.entity, this };

		// De-select
		if (selected.HasComponent<SelectedComponent>())
		{
			selected.RemoveComponent<SelectedComponent>();
			SetSelectedEntity(entt::null);
			return;
		}

		// Remove selection from all other entities
		auto view = m_Registry.view<SelectedComponent>();
		for (auto eRaw : view)
			Entity{ eRaw, this }.RemoveComponent<SelectedComponent>();

		selected.AddComponent<SelectedComponent>();

		return;
	}

	constexpr float MAX_DRAG_DISTANCE = 250.0f;
	if (m_IsDragging &&
	input.IsMousePressed(Mouse::Left) &&
	m_DraggedEntity != entt::null)
	{
		Entity dragged{ m_DraggedEntity, this };


		// Initialize drag state on first frame
		if (!m_DragDistanceLocked)
		{
			glm::vec3 camPos = cc.Camera.GetPosition();
			m_LockedDragDistance = glm::length(dragged.GetPosition() - camPos);
			m_LockedDragDistance = glm::clamp(m_LockedDragDistance, 2.0f, MAX_DRAG_DISTANCE);

			m_InitialDragPos = dragged.GetPosition();
			m_InitialMousePos = input.GetMousePos();
			m_DragDistanceLocked = true;

			m_FinalDragPos = m_InitialDragPos;

		}

		glm::vec2 mouseDelta = input.GetMousePos() - m_InitialMousePos;
		glm::vec3 newPos = m_InitialDragPos;


		// Screen-space sensitivity: adjust based on FOV and distance
		glm::vec2 viewport = m_Window.GetViewport();
		float fovScale = glm::tan(glm::radians(m_CameraProps.Fov * 0.5f));
		float sensitivity = (m_LockedDragDistance * fovScale) / (viewport.y * 0.5f);

		if (m_DragAffectsVertical)
		{
			glfwSetCursor(m_Window.GetGLFWwindow(), m_ResizeUpDownCursor);

			// Vertical drag mode: mouse Y controls Y axis
			newPos.y -= mouseDelta.y * sensitivity;
		}
		else if (m_DragAffectsXZ)
		{
			glfwSetCursor(m_Window.GetGLFWwindow(), m_MoveCursor);


			// XZ plane mode: mouse controls horizontal plane
			glm::vec3 camRight = cc.Camera.GetRightVector();
			glm::vec3 camForward = glm::normalize(glm::vec3(cc.Camera.GetForwardVector().x, 0.0f, cc.Camera.GetForwardVector().z));

			newPos += camRight * mouseDelta.x * sensitivity;
			newPos += camForward * (-mouseDelta.y) * sensitivity;


			m_LocalGridHeight = ComputeEntityFloorY(dragged);
			m_LocalGridSize = ComputeXZRadius(dragged);
			m_LocalGridOrigin = glm::vec3(newPos.x, m_LocalGridHeight, newPos.z);
		}

		auto& dtc = dragged.GetComponent<TransformComponent>();
		dtc.Translation = newPos;
		dtc.MarkDirty();
		m_FinalDragPos = newPos;
	}

	if (m_IsDragging && !input.IsMousePressed(Mouse::Left))
	{
		glfwSetCursor(m_Window.GetGLFWwindow(), m_OpenHandCursor);

		if (m_DraggedEntity != entt::null)
		{
			Entity dragged{ m_DraggedEntity, this };
			if (dragged)
			{
				glm::vec3 endPos = dragged.GetComponent<TransformComponent>().Translation;

				if (endPos != m_InitialDragPos)
					PushAction(MoveAction{ dragged.GetUUID(), m_InitialDragPos, endPos });
			}
		}

		m_IsDragging = false;
		m_DraggedEntity = entt::null;
		m_DragOffset = glm::vec3(0.0f);
		m_ShouldDrawLocalGrid = false;
		m_LocalGridID = entt::null;
		m_DragDistanceLocked = false;
	}

}

void Scene::DrawScreenOverlays(const CameraComponent& cc, Renderer& renderer)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glDepthMask(GL_TRUE);

	if (m_ShowCrosshair)
	{
		m_CrosshairShader->Bind();
		m_CrosshairShader->SetVec3("u_Color", CROSSHAIR_COLOR);
		m_CrosshairVA.Bind();
		glLineWidth(10.0f);
		if (!m_CursorEnabled)
			glDrawArrays(GL_LINES, 0, 4);
	}
}

void Scene::DrawGrid(const CameraComponent& cc) const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glBindVertexArray(m_InfiniteGridVAO);

	m_InfiniteGridShader->Bind();
	m_InfiniteGridShader->SetFloat("GridHeight", 0.0f);
	m_InfiniteGridShader->SetMat4("Projection", cc.Camera.GetProjectionMatrix());
	m_InfiniteGridShader->SetMat4("View", cc.Camera.GetViewMatrix());
	m_InfiniteGridShader->SetVec3("CameraWorldPos", cc.Camera.GetPosition());
	m_InfiniteGridShader->SetBool("ShowAxes", m_ShowAxes);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Scene::DrawLocalGrid(const CameraComponent& cc)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glm::vec3 origin = m_LocalGridOrigin;
	float h = m_LocalGridHeight;

	glBindVertexArray(m_GridVAO);

	m_GridShader->Bind();
	m_GridShader->SetMat4("Projection", cc.Camera.GetProjectionMatrix());
	m_GridShader->SetMat4("View",       cc.Camera.GetViewMatrix());
	m_GridShader->SetVec3("CameraWorldPos", origin);
	m_GridShader->SetFloat("GridHeight",    h);
	m_GridShader->SetFloat("gGridSize",     m_LocalGridSize);
	m_GridShader->SetFloat("gGridCellSize", 1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Scene::DrawSkybox(const CameraComponent& cc) const
{
	if (!m_SkyboxLoaded) return;

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	m_SkyboxShader->Bind();

	glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(cc.Camera.GetViewMatrix()));
	m_SkyboxShader->SetMat4("u_Projection", cc.Camera.GetProjectionMatrix());
	m_SkyboxShader->SetMat4("u_View", viewNoTranslate);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_HDRSkyboxCubemap);
	m_SkyboxShader->SetInt("u_Skybox", 0);
	m_SkyboxShader->SetFloat("u_Exposure", 1.5f);

	m_SkyboxVA.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
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


static bool RayIntersectsSphere(const Ray& ray, const glm::vec3& center, float radius, float& tOut)
{

	glm::vec3 oc = ray.Origin - center;

	float b = glm::dot(oc, ray.Direction);
	float c = glm::dot(oc, oc) - radius * radius;

	float disc = b * b - c;
	if (disc < 0.0f) return false;

	float s = sqrt(disc);
	float t0 = -b - s;
	float t1 = -b + s;

	float t = (t0 > 0.0f) ? t0 : t1;
	if (t <= 0.0f) return false;

	tOut = t;
	return true;
}

bool Scene::Raycast(const Ray& ray, RayHit& outHit) const
{
    bool hitAnything = false;
	const glm::vec3 rayDirN = glm::normalize(ray.Direction);

    auto view = m_Registry.view<TransformComponent, MeshComponent>();
    for (auto e : view)
    {
        const auto& tc = view.get<TransformComponent>(e);
        const auto& mc = view.get<MeshComponent>(e);
        if (!mc.MeshData) continue;

        // Cached bounds
        GPUMesh& gpu = MeshRendererCache::GetOrCreate(*mc.MeshData);

        if (!gpu.BoundsReady)
        {
            glm::vec3 c;
            float r = MeshUtils::CalculateMeshRadius(*mc.MeshData, c);
            gpu.LocalBoundsCenter = c;
            gpu.LocalBoundsRadius = r;
            gpu.BoundsReady = true;
        }

        // World-space sphere early-out (lets us compare against outHit.t)
        glm::vec3 centerWorld = tc.Translation; // fallback if cache isn't valid for some reason
        float radiusWorld = gpu.LocalBoundsRadius * glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

    	const auto& X = tc.GetCache();
    	const glm::mat4& model = X.Model;
    	const glm::mat4& invModel = X.InvModel;
    	const glm::mat3& normalMat = X.NormalMat;

        centerWorld = glm::vec3(model * glm::vec4(gpu.LocalBoundsCenter, 1.0f));

        float tSphereWorld = 0.0f;
        if (!RayIntersectsSphere(ray, centerWorld, radiusWorld, tSphereWorld))
            continue;

        if (tSphereWorld >= outHit.t)
            continue;

        // local ray
        Ray localRay;
        localRay.Origin = glm::vec3(invModel * glm::vec4(ray.Origin, 1.0f));
        localRay.Direction = glm::normalize(glm::vec3(invModel * glm::vec4(ray.Direction, 0.0f)));

        // Cached bounds
        float tSphere = 0.0f;
        if (!RayIntersectsSphere(localRay, gpu.LocalBoundsCenter, gpu.LocalBoundsRadius, tSphere))
            continue;

        const auto& verts = mc.MeshData->Vertices;
        const auto& inds  = mc.MeshData->Indices;

        for (size_t i = 0; i < inds.size(); i += 3)
        {
            glm::vec3 v0 = verts[inds[i+0]].Position;
            glm::vec3 v1 = verts[inds[i+1]].Position;
            glm::vec3 v2 = verts[inds[i+2]].Position;

            float tLocal;
            glm::vec3 nLocal;


        	if (RayIntersectsTriangle(localRay, v0, v1, v2, tLocal, nLocal))
        	{
        		// Local hit point
        		glm::vec3 pLocal = localRay.At(tLocal);

        		// World hit point
        		glm::vec3 pWorld = glm::vec3(model * glm::vec4(pLocal, 1.0f));

        		// Compute world t along world ray so we can compare across entities
        		float tWorld = glm::dot(pWorld - ray.Origin, rayDirN);

        		if (tWorld > 0.0f && tWorld < outHit.t)
        		{
        			outHit.Hit      = true;
        			outHit.t        = tWorld;
        			outHit.position = pWorld;
        			outHit.normal = glm::normalize(normalMat * nLocal);
        			outHit.entity = e;
        			hitAnything   = true;
        		}
        	}
        }
    }

    return hitAnything;
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

	// Store the part entities so we can sync transforms later
	auto& rootModel = root.AddComponent<ModelRootComponent>();

	for (auto& sm : model->Submeshes)
	{
		UUID partID;
		Entity part = CreateEntity(partID, rootName + "_Part");
		rootModel.Parts.push_back(partID);

		auto& partTC = part.GetComponent<TransformComponent>();
		partTC.Translation = rootTC.Translation;
		partTC.Rotation    = rootTC.Rotation;
		partTC.Scale       = rootTC.Scale;
		partTC.MarkDirty();

		auto& mc = part.AddComponent<MeshComponent>();
		mc.MeshData = sm.Mesh->MeshData;

		// Create per-entity material instance from imported submesh material
		auto& matc = part.AddComponent<MaterialComponent>();
		matc.Desc = sm.Material ? sm.Material->Desc : MaterialDesc{};
	}

	return root;
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
		auto root = Entity{e, this};
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

void Scene::BindMaterial(const MaterialComponent& material, const Ref<Shader>& shader, int& slot)
{
	const auto& desc = material.Desc;
	auto bindTex = [&](const char* uniform, AssetHandle h, GLuint fallback)
	{
		GLuint id = fallback;

		if (h != 0)
		{
			auto texAsset = AssetManager::GetAsset<TextureAsset>(h);
			if (texAsset && texAsset->Texture)
			{
				GLuint candidate = texAsset->Texture->GetID();
				if (candidate != 0 && glIsTexture(candidate))
					id = candidate;
			}
		}

		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);
		shader->SetInt(uniform, slot);
		slot++;
	};

	bindTex("u_BaseColorTex",         desc.BaseColorTexture,         m_WhiteTexture);
	bindTex("u_NormalTex",            desc.NormalTexture,            m_FlatNormalTexture);
	bindTex("u_MetallicRoughnessTex", desc.MetallicRoughnessTexture, m_WhiteTexture);
	bindTex("u_EmissiveTex",          desc.EmissiveTexture,          m_WhiteTexture);
	bindTex("u_OcclusionTex",         desc.OcclusionTexture,         m_WhiteTexture);

	shader->SetVec4 ("u_BaseColorFactor", desc.BaseColorFactor);
	shader->SetFloat("u_MetallicFactor",  desc.MetallicFactor);
	shader->SetFloat("u_RoughnessFactor", desc.RoughnessFactor);
	shader->SetVec3 ("u_EmissiveFactor",  desc.EmissiveFactor);
}

void Scene::SetModelMaterial(Entity root, AssetHandle matHandle)
{
	if (!root || !root.HasComponent<ModelRootComponent>()) return;

	auto mat = AssetManager::GetAsset<MaterialAsset>(matHandle);
	if (!mat) return;

	auto& mr = root.GetComponent<ModelRootComponent>();
	for (UUID id : mr.Parts)
	{
		Entity part = GetEntityByID(id);
		if (!part) continue;

		if (!part.HasComponent<MaterialComponent>())
			part.AddComponent<MaterialComponent>();

		auto& matc = part.GetComponent<MaterialComponent>();
		matc.BaseMaterial = matHandle;
		matc.Desc = mat->Desc; // seed instance
	}
}
glm::vec3 Scene::ColorFromTemperature(float kelvin)
{
	kelvin = glm::clamp(kelvin, 1000.0f, 40000.0f) / 100.0f;

	float r, g, b;

	// Red
	if (kelvin <= 66.0f)
		r = 255.0f;
	else {
		r = kelvin - 60.0f;
		r = 329.698727446f * pow(r, -0.1332047592f);
	}

	// Green
	if (kelvin <= 66.0f) {
		g = kelvin;
		g = 99.4708025861f * log(g) - 161.1195681661f;
	} else {
		g = kelvin - 60.0f;
		g = 288.1221695283f * pow(g, -0.0755148492f);
	}

	// Blue
	if (kelvin >= 66.0f)
		b = 255.0f;
	else if (kelvin <= 19.0f)
		b = 0.0f;
	else {
		b = kelvin - 10.0f;
		b = 138.5177312231f * log(b) - 305.0447927307f;
	}

	return glm::vec3(r, g, b) / 255.0f;
}

float Scene::ComputeEntityFloorY(Entity e)
{
	auto& tc = e.GetComponent<TransformComponent>();
	auto& mc = e.GetComponent<MeshComponent>();

	if (!mc.MeshData)
		return tc.Translation.y;

	float minY = std::numeric_limits<float>::max();
	const auto& verts = mc.MeshData->Vertices;

	const glm::mat4& model = tc.GetCache().Model;

	for (size_t i = 0; i < verts.size(); i++)
	{
		glm::vec3 v = verts[i].Position;
		glm::vec3 w = glm::vec3(model * glm::vec4(v, 1.0f));
		minY = std::min(minY, w.y);
	}

	return minY;
}

float Scene::ComputeXZRadius(Entity e)
{
	auto& tc = e.GetComponent<TransformComponent>();
	auto& mc = e.GetComponent<MeshComponent>();
	auto& mesh = *mc.MeshData;

	glm::mat4 M = tc.GetModelMatrix();

	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minZ = FLT_MAX, maxZ = -FLT_MAX;

	for (auto& v : mesh.Vertices)
	{
		glm::vec4 world = M * glm::vec4(v.Position, 1.0f);
		minX = std::min(minX, world.x);
		maxX = std::max(maxX, world.x);
		minZ = std::min(minZ, world.z);
		maxZ = std::max(maxZ, world.z);
	}

	float extentX = 0.5f * (maxX - minX);
	float extentZ = 0.5f * (maxZ - minZ);

	return std::max(extentX, extentZ);
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

static bool SnapshotContainsUUID(const EntitySnapshot& s, UUID id)
{
	if (!id) return false;
	if (s.id == id) return true;

	for (const auto& p : s.parts)
		if (SnapshotContainsUUID(p, id))
			return true;

	return false;
}


void Scene::PushAction(Action a)
{
	m_Undo.push_back(std::move(a));
	m_Redo.clear();
}

void Scene::ApplyUndo(const MoveAction& a)
{
	Entity e = GetEntityByID(a.id);
	if (!e) return;
	auto& tc = e.GetComponent<TransformComponent>();
	tc.Translation = a.before;
	tc.MarkDirty();
}

void Scene::ApplyRedo(const MoveAction& a)
{
	Entity e = GetEntityByID(a.id);
	if (!e) return;
	auto& tc = e.GetComponent<TransformComponent>();
	tc.Translation = a.after;
	tc.MarkDirty();
}

void Scene::ApplyUndo(const CreateAction& a)
{
	if (SnapshotContainsUUID(a.snapshot, m_SelectedUUID))
		SetSelectedEntity(entt::null);

	DeleteSnapshotRecursive(this, a.snapshot);
}

void Scene::ApplyRedo(const CreateAction& a)
{
	RestoreEntityFromSnapshot(a.snapshot);
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

Entity Scene::RestoreEntityFromSnapshot(const EntitySnapshot& s)
{
	// If it already exists, delete it first to avoid UUID collisions in maps
	{
		Entity existing = GetEntityByID(s.id);
		if (existing) DeleteEntity(existing);
	}

	// Create entity with the same UUID and tag
	Entity e = CreateEntity(s.id, s.tag);

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
		// Ensure root has ModelRootComponent
		if (!e.HasComponent<ModelRootComponent>())
			e.AddComponent<ModelRootComponent>();

		auto& mr = e.GetComponent<ModelRootComponent>();
		mr.Parts.clear();
		mr.Parts.reserve(s.parts.size());

		// Restore parts as separate entities, then register UUIDs in mr.Parts
		for (const EntitySnapshot& ps : s.parts)
		{
			Entity part = RestoreEntityFromSnapshot(ps);
			if (part)
				mr.Parts.push_back(part.GetUUID());
		}

		// Initialize last fields so SyncSubmeshes doesn't instantly overwrite children.
		mr.LastTranslation = e.GetComponent<TransformComponent>().Translation;
		mr.LastRotation    = e.GetComponent<TransformComponent>().Rotation;
		mr.LastScale       = e.GetComponent<TransformComponent>().Scale;
	}

	// Never restore selection state from undo snapshots.
	if (e.HasComponent<SelectedComponent>())
		e.RemoveComponent<SelectedComponent>();

	return e;
}


void Scene::ApplyUndo(const DeleteAction& a)
{
	RestoreEntityFromSnapshot(a.snapshot);
}

void Scene::ApplyRedo(const DeleteAction& a)
{
	if (SnapshotContainsUUID(a.snapshot, m_SelectedUUID))
		SetSelectedEntity(entt::null);

	Entity e = GetEntityByID(a.snapshot.id);
	if (e) DeleteEntity(e);
}

void Scene::Undo()
{
	if (m_Undo.empty()) return;

	Action a = std::move(m_Undo.back());
	m_Undo.pop_back();

	std::visit([&](auto& act){ ApplyUndo(act); }, a);

	m_Redo.push_back(std::move(a));
}

void Scene::Redo()
{
	if (m_Redo.empty()) return;

	Action a = std::move(m_Redo.back());
	m_Redo.pop_back();

	std::visit([&](auto& act){ ApplyRedo(act); }, a);

	m_Undo.push_back(std::move(a));
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

Entity Scene::GetEntityByName(const std::string& name)
{
	auto it = m_EntityNameMap.find(name);
	if (it != m_EntityNameMap.end())
		return Entity{ it->second, this };

	return {};
}

Entity Scene::GetEntityByID(UUID id)
{
	auto it = m_EntityMap.find(id);
	if (it != m_EntityMap.end())
		return Entity{ it->second, this };

	return {};
}

float Scene::CalculateEntityRadius(Entity entity)
{
	// Single mesh entity
	if (entity.HasComponent<MeshComponent>() && !entity.HasComponent<ModelRootComponent>())
	{
		auto& mc = entity.GetComponent<MeshComponent>();
		if (mc.MeshData)
		{
			glm::vec3 center;
			float localRadius = MeshUtils::CalculateMeshRadius(*mc.MeshData, center);

			// Account for scale transformation
			auto& tc = entity.GetComponent<TransformComponent>();
			float maxScale = glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

			return localRadius * maxScale;
		}
		return 5.0f;
	}

	// Multi-part model
	if (entity.HasComponent<ModelRootComponent>())
	{
		float maxRadius = 0.0f;
		auto& parts = entity.GetComponent<ModelRootComponent>().Parts;

		for (UUID id : parts)
		{
			Entity part = GetEntityByID(id);
			if (!part || !part.HasComponent<MeshComponent>())
				continue;

			auto& mc = part.GetComponent<MeshComponent>();
			if (!mc.MeshData)
				continue;

			glm::vec3 meshCenter;
			float localRadius = MeshUtils::CalculateMeshRadius(*mc.MeshData, meshCenter);

			auto& tc = part.GetComponent<TransformComponent>();
			float maxScale = glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

			maxRadius = glm::max(maxRadius, localRadius * maxScale);
		}

		return maxRadius > 0.0f ? maxRadius : 5.0f;
	}

	return 5.0f;
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

	if (!name.empty() && !m_EntityNameMap.contains(tag.Tag))
		m_EntityNameMap[tag.Tag] = entity; // Used for O(1) entity lookup

	return entity;
}

void Scene::DeleteEntity(Entity entity)
{
	m_EntityMap.erase(entity.GetUUID());
	m_EntityNameMap.erase(entity.GetName());
	m_Registry.destroy(entity);
}

Entity Scene::DuplicateEntity(Entity src)
{
    if (!src) return {};

    // Create fresh entity
	std::string srcTag = src.GetTag();

	// Strip trailing " [Copy N]" if present.
	std::string base = srcTag;
	{
    	int i = (int)base.size() - 1;

    	if (i >= 0 && base[i] == ']')
    	{
    		i--; // before ']'
    		while (i >= 0 && std::isdigit((unsigned char)base[i])) i--;

    		const std::string needle = "[Copy ";
    		int start = i - (int)needle.size() + 1;

    		if (start >= 0 &&
				(i + 1) < (int)base.size() && // there was at least 1 digit
				base.compare((size_t)start, needle.size(), needle) == 0 &&
				start > 0 && base[start - 1] == ' ')
    		{
    			// Remove " [Copy N]"
    			base.erase((size_t)(start - 1));
    		}
    	}
	}

	int nextIndex = 1;
	{
    	const std::string prefix = base + " [Copy ";
    	auto view = m_Registry.view<TagComponent>();

    	for (auto e : view)
    	{
    		const std::string& name = view.get<TagComponent>(e).Tag;
    		if (name.rfind(prefix, 0) != 0) continue;

    		size_t pos = prefix.size();
    		int val = 0;
    		bool any = false;

    		while (pos < name.size() && std::isdigit((unsigned char)name[pos]))
    		{
    			any = true;
    			val = val * 10 + (name[pos] - '0');
    			pos++;
    		}

    		if (!any) continue;
    		if (pos >= name.size() || name[pos] != ']') continue;
    		if (pos + 1 != name.size()) continue;

    		nextIndex = std::max(nextIndex, val + 1);
    	}
	}

	std::string newName = base + " [Copy " + std::to_string(nextIndex) + "]";
    Entity dst = CreateEntity(UUID(), newName);

    // Copy component if present (add if missing, else assign)
    auto copyOrAssign = [&](auto tagType)
    {
        using T = decltype(tagType);
        if (!src.HasComponent<T>()) return;

        if (dst.HasComponent<T>())
            dst.GetComponent<T>() = src.GetComponent<T>();
        else
            dst.AddComponent<T>(src.GetComponent<T>());
    };

    // Copy transform
	if (src.HasComponent<TransformComponent>())
	{
		dst.GetComponent<TransformComponent>() = src.GetComponent<TransformComponent>();
		dst.GetComponent<TransformComponent>().MarkDirty();
	}

    // Copy components that are safe to shallow-copy
    copyOrAssign(MeshComponent{});
	copyOrAssign(MaterialComponent{});
    copyOrAssign(LightComponent{});

    // Don't copy selection by default
    if (dst.HasComponent<SelectedComponent>())
        dst.RemoveComponent<SelectedComponent>();

    // Deep-copy model roots
    if (src.HasComponent<ModelRootComponent>())
    {
        const auto& srcRoot = src.GetComponent<ModelRootComponent>();

        if (!dst.HasComponent<ModelRootComponent>())
            dst.AddComponent<ModelRootComponent>();

        auto& dstRoot = dst.GetComponent<ModelRootComponent>();
        dstRoot.Parts.clear();

        for (UUID partID : srcRoot.Parts)
        {
            Entity srcPart = GetEntityByID(partID);
            if (!srcPart) continue;

            Entity dstPart = CreateEntity(UUID(), newName + "_Part");

            // Copy part transform + mesh
        	if (srcPart.HasComponent<TransformComponent>())
        	{
        		dstPart.GetComponent<TransformComponent>() = srcPart.GetComponent<TransformComponent>();
        		dstPart.GetComponent<TransformComponent>().MarkDirty();
        	}

            if (srcPart.HasComponent<MeshComponent>())
                dstPart.AddComponent<MeshComponent>(srcPart.GetComponent<MeshComponent>());

        	if (srcPart.HasComponent<MaterialComponent>())
        		dstPart.AddComponent<MaterialComponent>(srcPart.GetComponent<MaterialComponent>());

            // Keep parts unselected
            if (dstPart.HasComponent<SelectedComponent>())
                dstPart.RemoveComponent<SelectedComponent>();

            dstRoot.Parts.push_back(dstPart.GetUUID());
        }

        // Initialize last fields so SyncSubmeshes doesn't immediately resync
        dstRoot.LastTranslation = dst.GetComponent<TransformComponent>().Translation;
        dstRoot.LastRotation    = dst.GetComponent<TransformComponent>().Rotation;
        dstRoot.LastScale       = dst.GetComponent<TransformComponent>().Scale;
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

Entity Scene::FindModelRootFromPart(Entity part)
{
	if (!part) return {};

	UUID partID = part.GetUUID();

	for (auto e : m_Registry.view<ModelRootComponent>())
	{
		Entity root{ e, this };
		const auto& mr = root.GetComponent<ModelRootComponent>();

		for (UUID id : mr.Parts)
			if (id == partID)
				return root;
	}

	return {};
}

void Scene::SetEntityMaterial(Entity e, AssetHandle matHandle)
{
	if (!e) return;

	auto mat = AssetManager::GetAsset<MaterialAsset>(matHandle);
	if (!mat) return;

	if (!e.HasComponent<MaterialComponent>())
		e.AddComponent<MaterialComponent>();

	auto& mi = e.GetComponent<MaterialComponent>();
	mi.BaseMaterial = matHandle;
	mi.Desc = mat->Desc;
}