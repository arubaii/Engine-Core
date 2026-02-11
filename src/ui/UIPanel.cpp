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

void UIPanel::Render(UI& data, entt::entity selected, Scene* scene)
{
    ImGui::Begin("UI Panel");

	if (ImGui::CollapsingHeader("Debug Data"))
	{
		static float fpsTimer = 0.0f;
		static int displayedFPS = 0;
		static float ftTimer = 0.0f;
		static float displayedFrameTime = 0.0f;
		float dt = ImGui::GetIO().DeltaTime;

		fpsTimer += dt;
		if (fpsTimer >= 0.60f)
		{
			displayedFPS = data.fps;
			fpsTimer = 0.0f;
		}

		ftTimer += dt;
		if (ftTimer >= 0.60f)
		{
			displayedFrameTime = data.frameTime;
			ftTimer = 0.0f;
		}
		ImGui::Text("FPS: %d", displayedFPS);
		ImGui::Text("Frame Time: %.3f ms", displayedFrameTime);

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


		if (ImGui::Button("Reveal Hidden Lights")) { UIUtils::StartLightsFlash(data.showLights); }
	}

	if (ImGui::CollapsingHeader("Selected Entity Config", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (selected != entt::null)
		{
			Entity e{ selected, scene };
			static entt::entity s_LastSelected = entt::null;
			static int materialIdx = (int)MATERIALS::None;

			// Reset certain config on new selected entity

			if (selected != s_LastSelected)
			{
				s_LastSelected = selected;

				if (e.HasComponent<MaterialComponent>())
				{
					auto& d = e.GetComponent<MaterialComponent>().Desc;

					// Try to match the current material to a preset
					materialIdx = (int)FindMatchingMaterialPreset(d);
				}
				else
				{
					materialIdx = (int)MATERIALS::None;
				}
			}

			if (e.HasComponent<SelectedComponent>())
			{
				auto& tc = e.GetComponent<TransformComponent>();

				glm::vec3 pos = tc.GetPosition();
				float scale = tc.Scale.x;
				float rotVertDeg  = glm::degrees(tc.Rotation.x); // pitch
				float rotHorizDeg = glm::degrees(tc.Rotation.y); // yaw

				// Keep values in a nice UI range
				if (rotVertDeg  > 180.0f) rotVertDeg  -= 360.0f;
				if (rotHorizDeg > 180.0f) rotHorizDeg -= 360.0f;


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

				ImGui::Text("Entity Position: (%.2f, %.2f, %.2f)",
						pos.x, pos.y, pos.z);

				// -------------------------------------------------------------------------------------

				ImGui::Dummy(ImVec2(0.0f, 8.0f)); ImGui::Separator(); ImGui::Dummy(ImVec2(0.0f, 8.0f));

				// -------------------------------------------------------------------------------------


				if (ImGui::SliderFloat("Scale Entity", &scale, 0.1f, 100.0f))
					tc.SetScale(glm::vec3(scale));


				if (ImGui::SliderFloat("Rotate Entity Horizontally", &rotHorizDeg, -180.0f, 180.0f))
				{
					glm::vec3 r = tc.Rotation;
					r.y = glm::radians(rotHorizDeg);
					tc.SetRotation(r);
				}

				if (ImGui::SliderFloat("Rotate Entity Vertically", &rotVertDeg, -89.0f, 89.0f))
				{
					glm::vec3 r = tc.Rotation;
					r.x = glm::radians(rotVertDeg);
					tc.SetRotation(r);
				}

				// -------------------------------------------------------------------------------------

				ImGui::Dummy(ImVec2(0.0f, 8.0f)); ImGui::Separator(); ImGui::Dummy(ImVec2(0.0f, 8.0f));

				// -------------------------------------------------------------------------------------


				if (!e.HasComponent<MaterialComponent>() && !e.HasComponent<LightComponent>())
				{


					auto& mc = e.GetComponent<MeshComponent>();
					ImGui::ColorEdit4("Base Color", &mc.BaseColor.x);

					if (ImGui::Checkbox("Use Normal Map Colors", &mc.UseNormalColors)) {}


					if (ImGui::Button("Add Material Component"))
						e.AddComponent<MaterialComponent>();
				}

				else if (e.HasComponent<MaterialComponent>())
				{
					auto& matc = e.GetComponent<MaterialComponent>();
					// Dropdown
					bool presetChanged = ImGui::Combo("Material Preset", &materialIdx,
					                                   GetMaterialDropdownItems(),
					                                   GetMaterialDropdownCount());

					MATERIALS mat = (MATERIALS)materialIdx;
					auto& d = e.GetComponent<MaterialComponent>().Desc;

					if ((mat != MATERIALS::None) && presetChanged)
					{
						auto spec = GetMaterialType(mat);
						d.BaseColorFactor = spec.BaseColorFactor;
						d.MetallicFactor  = spec.MetallicFactor;
						d.RoughnessFactor = spec.RoughnessFactor;
						d.LightBoostFactor = spec.LightBoostFactor;
					}

					ImGui::ColorEdit4("BaseColor", &d.BaseColorFactor.x);
					ImGui::SliderFloat("Metallic",  &d.MetallicFactor,  0.0f, 1.0f);
					ImGui::SliderFloat("Roughness", &d.RoughnessFactor, 0.0f, 1.0f);
					ImGui::ColorEdit3("Emissive",   &d.EmissiveFactor.x);

					if (ImGui::SliderFloat("Emissive Strength", &matc.Desc.EmissiveStrength, 0.1f, 100.0f))
					{}
					if (ImGui::SliderFloat("Light Strength", &matc.Desc.LightBoostFactor, 0.1f, 1000.0f))
					{}


					if (ImGui::Button("Remove Material Component"))
						e.RemoveComponent<MaterialComponent>();
				}
			}

			if (e.HasComponent<LightComponent>())
			{
				auto& lc = e.GetComponent<LightComponent>();
				float luminosity = lc.Luminosity;
				float temp = lc.Temperature;
				if (ImGui::SliderFloat("Luminosity", &luminosity, 50.0f, 50000.0f))
					lc.Luminosity = luminosity;
				if (ImGui::SliderFloat("Temperature (Kelvin)", &temp, 1000.0f, 20000.0f))
					lc.Temperature = temp;
				ImGui::ColorEdit3("Tint Color", glm::value_ptr(lc.TintColor));

				if (ImGui::Checkbox("Hide Light", &lc.HideLight)) {}
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