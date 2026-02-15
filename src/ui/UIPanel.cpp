#include "UIPanel.h"
#include "utils/Log.h"


static int FindMatchingMaterialPreset(const MaterialDesc& d)
{
	for (int i = 0; i < (int)MATERIALS::Count; i++)
	{
		MATERIALS preset = (MATERIALS)i;
		auto spec = GetMaterialType(preset);

		bool matches =
			glm::all(glm::epsilonEqual(d.BaseColorFactor, spec.BaseColorFactor, 0.001f)) &&
			abs(d.MetallicFactor  - spec.MetallicFactor)  < 0.001f &&
			abs(d.RoughnessFactor - spec.RoughnessFactor) < 0.001f;

		if (matches)
			return i;
	}

	return (int)MATERIALS::None;
}


const int UIPanel::s_MaxSamples = 120;
float UIPanel::s_FPSHistory[120] = {};
int   UIPanel::s_FPSOffset = 0;
float UIPanel::s_RunningSum = 0.0f;
int   UIPanel::s_SampleCount = 0;
float UIPanel::smoothedFPS = 0.0f;

void UIPanel::Render(UI& data, entt::entity selected, Scene* scene)
{
    ImGui::Begin("UI Panel");

	if (ImGui::CollapsingHeader("Debug Data"))
	{
		const float smoothing = 0.10f;   // lower is smoother

		static float smoothedFrameTime = 0.0f;

		if (smoothedFrameTime == 0.0f)
			smoothedFrameTime = data.frameTime;
		else
			smoothedFrameTime =
				smoothing * data.frameTime +
				(1.0f - smoothing) * smoothedFrameTime;


		s_RunningSum -= s_FPSHistory[s_FPSOffset];

		// Insert new smoothed frame time
		s_FPSHistory[s_FPSOffset] = smoothedFrameTime;
		s_RunningSum += s_FPSHistory[s_FPSOffset];


		s_FPSOffset = (s_FPSOffset + 1) % s_MaxSamples;


		if (s_SampleCount < s_MaxSamples)
			s_SampleCount++;


		float avgFrameTime =
			s_SampleCount > 0
				? s_RunningSum / s_SampleCount
				: 0.0f;



		// Overlay text
		char overlay[32];
		snprintf(overlay, sizeof(overlay),
				 "Avg %.0f FPS (%.2f ms)",
				 avgFrameTime > 0.0f ? 1000.0f / avgFrameTime : 0.0f,
				 avgFrameTime);

		// Stable graph range (16.67 ms = 60 FPS)
		float graphMax = 20.0f;
		ImGui::PlotLines(
			"##FrameTimeGraph",
			s_FPSHistory,
			s_SampleCount,
			s_FPSOffset,
			overlay,
			0.0f,
			graphMax,
			ImVec2(0, 80)
		);

		ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
					data.cameraPos.x, data.cameraPos.y, data.cameraPos.z);
		ImGui::Text("Camera View (Pitch, Yaw): (%.2f, %.2f)",
					data.pitch, data.yaw);

		if (ImGui::Button("Show Crosshair")) data.showCrosshair = !data.showCrosshair;
	}

	if (ImGui::CollapsingHeader("Scene Config"))
	{
		ImGui::PushItemWidth(70);
		ImGui::SliderFloat("Background Greyscale", &data.greyScale, 0.0f, 1.0f, "%.2f");
		ImGui::PopItemWidth();
		if (ImGui::Button("Show Grid"))          data.showGrid     = !data.showGrid;
		if (ImGui::Button("Show Grid Axes"))     data.showAxes	   = !data.showAxes;
		if (ImGui::Button("Show Skybox"))		 data.showSkybox   = !data.showSkybox;
		if (ImGui::Checkbox("Enable Physics",    &data.enablePhysics)) {}

		if (ImGui::Button("Reveal Hidden Lights")) { UIUtils::StartLightsFlash(data.showLights); }
	}

	if (ImGui::CollapsingHeader("Selected Entity Config", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (selected != entt::null)
		{
			Entity e{ selected, &scene->GetRegistry() };

			Entity materialOwner = e;
			MaterialComponent* matPtr = nullptr;

			if (e.HasComponent<MaterialComponent>())
			{
				matPtr = &e.GetComponent<MaterialComponent>();
			}
			else if (e.HasComponent<ModelRootComponent>())
			{
				auto& root = e.GetComponent<ModelRootComponent>();
				if (!root.Parts.empty())
				{
					Entity part = scene->GetEntityByID(root.Parts[0]);
					if (part && part.HasComponent<MaterialComponent>())
					{
						materialOwner = part;
						matPtr = &part.GetComponent<MaterialComponent>();
					}
				}
			}

			static entt::entity s_LastSelected = entt::null;
			static int materialIdx = (int)MATERIALS::None;

			static bool s_EditingMaterial = false;
			static UUID s_EditMaterialID = {};
			static MaterialComponent s_BeforeMaterial;

			static bool s_EditingLight = false;
			static UUID s_EditLightID = {};
			static LightComponent s_BeforeLight;

			// Reset certain config on new selected entity

			if (selected != s_LastSelected)
			{
				s_LastSelected = selected;

				if (matPtr)
				{
					// Try to match the current material to a preset
					materialIdx = (int)FindMatchingMaterialPreset(matPtr->Desc);
				}
				else
				{
					materialIdx = (int)MATERIALS::None;
				}

				s_EditingMaterial = false;
				s_EditMaterialID = {};
				s_EditingLight = false;
				s_EditLightID = {};
			}

			if (e.HasComponent<SelectedComponent>() &&
				e.HasComponent<TagComponent>() &&
				e.HasComponent<TransformComponent>())
			{
				auto& tag = e.GetComponent<TagComponent>();
				auto& tc  = e.GetComponent<TransformComponent>();

				size_t triangleCount = 0;

				if (e.HasComponent<MeshComponent>())
				{
					auto& meshData = e.GetComponent<MeshComponent>().MeshData;
					if (meshData)
						triangleCount = meshData->Indices.size() / 3;
				}
				else if (e.HasComponent<ModelRootComponent>())
				{
					auto& root = e.GetComponent<ModelRootComponent>();
					for (UUID id : root.Parts)
					{
						Entity part = scene->GetEntityByID(id);
						if (part && part.HasComponent<MeshComponent>())
						{
							auto& meshData = part.GetComponent<MeshComponent>().MeshData;
							if (meshData)
								triangleCount += meshData->Indices.size() / 3;
						}
					}
				}

				glm::vec3 pos = tc.GetPosition();
				float scale = tc.Scale.x;
				float rotVertDeg  = glm::degrees(tc.Rotation.x); // pitch
				float rotHorizDeg = glm::degrees(tc.Rotation.y); // yaw

				// Keep values in a nice UI range
				if (rotVertDeg  > 180.0f) rotVertDeg  -= 360.0f;
				if (rotHorizDeg > 180.0f) rotHorizDeg -= 360.0f;

				ImGui::Text("Entity: %s", tag.Tag.c_str());
				ImGui::Text("Entity Triangles %zu:", triangleCount);
				ImGui::Text("Entity Position: (%.2f, %.2f, %.2f)",
						pos.x, pos.y, pos.z);

				if (ImGui::Checkbox("Drag Affects Vertical (Y)", &data.dragAffectsVertical))
				{
					if (data.dragAffectsVertical)
						data.dragAffectsXZ = false;
				}

				if (ImGui::Checkbox("Drag Affects XZ Plane", &data.dragAffectsXZ))
				{
					if (data.dragAffectsXZ)
						data.dragAffectsVertical = false;
				}

				ImGui::Dummy(ImVec2(0.0f, 8.0f)); ImGui::Separator(); ImGui::Dummy(ImVec2(0.0f, 8.0f));

				if (ImGui::SliderFloat("Scale Entity", &scale, 0.1f, 100.0f))
					tc.SetScale(glm::vec3(scale));

				if (ImGui::SliderFloat("Rotate Entity Horizontally", &rotHorizDeg, -180.0f, 180.0f))
				{
					glm::vec3 r = tc.Rotation;
					r.y = glm::radians(rotHorizDeg);
					tc.SetRotation(r);
				}

				if (ImGui::SliderFloat("Rotate Entity Vertically", &rotVertDeg, -180.0f, 180.0f))
				{
					glm::vec3 r = tc.Rotation;
					r.x = glm::radians(rotVertDeg);
					tc.SetRotation(r);
				}

				ImGui::Dummy(ImVec2(0.0f, 8.0f)); ImGui::Separator(); ImGui::Dummy(ImVec2(0.0f, 8.0f));

				if (e.HasComponent<MeshComponent>() &&
					!e.HasComponent<MaterialComponent>() &&
					!e.HasComponent<LightComponent>())
				{
					auto& mc = e.GetComponent<MeshComponent>();
					ImGui::ColorEdit4("Base Color", &mc.BaseColor.x);

					if (ImGui::Checkbox("Use Normal Map Colors", &mc.UseNormalColors)) {}

					if (ImGui::Button("Add Material Component"))
					{
						auto& matc = e.AddComponent<MaterialComponent>();
						if (mc.Material)
						{
							matc.BaseMaterial = mc.Material->Handle;
							matc.Desc = mc.Material->Desc;
						}
						else
						{
							matc.BaseMaterial = UUID(0);
							matc.Desc = MaterialDesc{};
						}
					}
				}
				else if (matPtr)
				{
					auto& matc = *matPtr;
					auto& d = matc.Desc;



					bool matItemActive = false;

					bool presetChanged = ImGui::Combo("Material Preset", &materialIdx,
					                                   GetMaterialDropdownItems(),
					                                   GetMaterialDropdownCount());
					matItemActive |= ImGui::IsItemActive();

					if ((MATERIALS)materialIdx != MATERIALS::None && presetChanged)
					{
						if (!s_EditingMaterial)
						{
							s_EditingMaterial = true;
							s_EditMaterialID = materialOwner.GetUUID();
							s_BeforeMaterial = matc;
						}

						auto spec = GetMaterialType((MATERIALS)materialIdx);
						d.BaseColorFactor = spec.BaseColorFactor;
						d.MetallicFactor  = spec.MetallicFactor;
						d.RoughnessFactor = spec.RoughnessFactor;
						d.LightBoostFactor = spec.LightBoostFactor;
					}

					if (ImGui::ColorEdit4("BaseColor", &d.BaseColorFactor.x)) {}
					if (!s_EditingMaterial && ImGui::IsItemActivated())
					{
						s_EditingMaterial = true;
						s_EditMaterialID = materialOwner.GetUUID();
						s_BeforeMaterial = matc;
					}
					matItemActive |= ImGui::IsItemActive();

					if (ImGui::SliderFloat("Metallic",  &d.MetallicFactor,  0.0f, 1.0f)) {}
					if (!s_EditingMaterial && ImGui::IsItemActivated())
					{
						s_EditingMaterial = true;
						s_EditMaterialID = materialOwner.GetUUID();
						s_BeforeMaterial = matc;
					}
					matItemActive |= ImGui::IsItemActive();

					if (ImGui::SliderFloat("Roughness", &d.RoughnessFactor, 0.0f, 1.0f)) {}
					if (!s_EditingMaterial && ImGui::IsItemActivated())
					{
						s_EditingMaterial = true;
						s_EditMaterialID = materialOwner.GetUUID();
						s_BeforeMaterial = matc;
					}
					matItemActive |= ImGui::IsItemActive();

					if (ImGui::ColorEdit3("Emissive",   &d.EmissiveFactor.x)) {}
					if (!s_EditingMaterial && ImGui::IsItemActivated())
					{
						s_EditingMaterial = true;
						s_EditMaterialID = materialOwner.GetUUID();
						s_BeforeMaterial = matc;
					}
					matItemActive |= ImGui::IsItemActive();

					if (ImGui::SliderFloat("Emissive Strength", &matc.Desc.EmissiveStrength, 0.1f, 100.0f)) {}
					if (!s_EditingMaterial && ImGui::IsItemActivated())
					{
						s_EditingMaterial = true;
						s_EditMaterialID = materialOwner.GetUUID();
						s_BeforeMaterial = matc;
					}
					matItemActive |= ImGui::IsItemActive();

					if (ImGui::SliderFloat("Light Strength", &matc.Desc.LightBoostFactor, 0.1f, 1000.0f)) {}
					if (!s_EditingMaterial && ImGui::IsItemActivated())
					{
						s_EditingMaterial = true;
						s_EditMaterialID = materialOwner.GetUUID();
						s_BeforeMaterial = matc;
					}
					matItemActive |= ImGui::IsItemActive();

					if (s_EditingMaterial &&
						s_EditMaterialID == materialOwner.GetUUID() &&
						!matItemActive)
					{
						ModifyMaterialAction action;
						action.id = s_EditMaterialID;
						action.before = s_BeforeMaterial;
						action.after = matc;
						scene->GetUndoSystem().Push(action);

						s_EditingMaterial = false;
						s_EditMaterialID = {};
					}

					if (ImGui::Button("Remove Material Component"))
						materialOwner.RemoveComponent<MaterialComponent>();
				}
			}

			if (e.HasComponent<LightComponent>())
			{
				auto& lc = e.GetComponent<LightComponent>();
				float luminosity = lc.Luminosity;
				float temp = lc.Temperature;

				bool lightItemActive = false;

				if (ImGui::SliderFloat("Luminosity", &luminosity, 50.0f, 50000.0f))
					lc.Luminosity = luminosity;
				if (!s_EditingLight && ImGui::IsItemActivated())
				{
					s_EditingLight = true;
					s_EditLightID = e.GetUUID();
					s_BeforeLight = lc;
				}
				lightItemActive |= ImGui::IsItemActive();

				if (ImGui::SliderFloat("Temperature (Kelvin)", &temp, 1000.0f, 20000.0f))
					lc.Temperature = temp;
				if (!s_EditingLight && ImGui::IsItemActivated())
				{
					s_EditingLight = true;
					s_EditLightID = e.GetUUID();
					s_BeforeLight = lc;
				}
				lightItemActive |= ImGui::IsItemActive();

				if (ImGui::ColorEdit3("Tint Color", glm::value_ptr(lc.TintColor))) {}
				if (!s_EditingLight && ImGui::IsItemActivated())
				{
					s_EditingLight = true;
					s_EditLightID = e.GetUUID();
					s_BeforeLight = lc;
				}
				lightItemActive |= ImGui::IsItemActive();

				if (ImGui::Checkbox("Hide Light", &lc.HideLight)) {}
				if (!s_EditingLight && ImGui::IsItemActivated())
				{
					s_EditingLight = true;
					s_EditLightID = e.GetUUID();
					s_BeforeLight = lc;
				}
				lightItemActive |= ImGui::IsItemActive();

				if (s_EditingLight &&
					s_EditLightID == e.GetUUID() &&
					!lightItemActive)
				{
					ModifyLightAction action;
					action.id = s_EditLightID;
					action.before = s_BeforeLight;
					action.after = lc;
					scene->GetUndoSystem().Push(action);

					s_EditingLight = false;
					s_EditLightID = {};
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Controls"))
	{
		ImGui::TextUnformatted("Controls:");
    	ImGui::Separator();

    	ImGui::BulletText("Escape: Enable / Disable Cursor");
    	ImGui::BulletText("Space: Move Upward / Zoom Out");
    	ImGui::BulletText("C: Move Upward / Zoom In");
    	ImGui::BulletText("WASD / Arrow Keys (In Orbit): Movement");
    	ImGui::BulletText("P or Middle Mouse (Hovering over entity): Enter / Exit orbit camera");

    	ImGui::Separator();

    	ImGui::BulletText("Left Click: Select / Deselect entity");
    	ImGui::BulletText("Shift + Left Click (drag): Drag selected entity");
    	ImGui::BulletText("Shift + K (Hovering over entity): Duplicate entity");
    	ImGui::BulletText("Shift + L (Hovering over entity): Delete entity");
    	ImGui::BulletText("Control or Cmd (Macos) + Z: Undo Action");
    	ImGui::BulletText("Control or Cmd (Macos) + Shift + Z: Redo Action");
	}

    ImGui::End();
}