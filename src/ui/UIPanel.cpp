#include "UIPanel.h"
#include "utils/Log.h"
#include <imgui/imgui_internal.h> // ui docking

#include "scene_core/ecs/EntityUtils.h"


const int	  UIPanel::s_MaxSamples = 120;
float 		  UIPanel::s_FPSHistory[120] = {};
int   		  UIPanel::s_FPSOffset = 0;
float 		  UIPanel::s_RunningSum = 0.0f;
int   		  UIPanel::s_SampleCount = 0;
float		  UIPanel::smoothedFPS = 0.0f;
EditorCursors UIPanel::s_Cursors;
EditorConfig  UIPanel::s_Config;
float 		  UIPanel::s_UiWidth     = 0.0f;
float 		  UIPanel::s_BottomHeight = 250.0f;

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

void UIPanel::SetCursors(GLFWwindow* glfwWindow)
{
	bool inUI = ImGui::GetIO().WantCaptureMouse;

	if (s_Config.IsSplitterDraggingX)
		glfwSetCursor(glfwWindow, s_Cursors.ResizeLeftRight);
	else if (s_Config.IsSplitterDraggingY)
		glfwSetCursor(glfwWindow, s_Cursors.ResizeUpDown);
	else if (s_Config.IsCornerDragging || s_Config.IsCornerHovered)
	{
		glfwSetCursor(glfwWindow, s_Cursors.ResizeDiagonal);
	}
	else if (inUI)
		glfwSetCursor(glfwWindow, s_Cursors.Arrow);
}

void UIPanel::Splitter(UIData& data,
                       EditorConfig& config,
                       GLFWwindow* glfwWindow,
                       bool vertical,
                       float thickness,
                       float* size0,
                       float* size1,
                       float min_size0,
                       float min_size1,
                       float totalWidth
)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window) return;

    const float hit = 12.0f;

    ImGui::PushID(vertical ? "UIPanelSplitterV" : "UIPanelSplitterH");

    ImVec2 winPos  = window->Pos;
    ImVec2 winSize = window->Size;

    if (vertical)
    {
        float x = winPos.x + winSize.x - thickness;

        ImRect bb(
            ImVec2(x - thickness * 0.5f, winPos.y),
            ImVec2(x + thickness * 0.5f, winPos.y + winSize.y)
        );

        ImGui::SetCursorPos(ImVec2(winSize.x - thickness - hit, 0.0f));
        ImGui::InvisibleButton("##SplitterV", ImVec2(thickness + hit, winSize.y));

        bool hovered = ImGui::IsItemHovered();
        bool active  = ImGui::IsItemActive();
        ImU32 barColor;
        if (active)
           barColor = IM_COL32(0, 120, 255, 255);
        else if (hovered)
           barColor = IM_COL32(255, 255, 255, 255);
        else
           barColor = ImGui::GetColorU32(ImGuiCol_Separator);

        window->DrawList->AddRectFilled(
          bb.Min, bb.Max,
          barColor
       );

        if (hovered || active)
        {
            config.IsSplitterDraggingX = true;
        }
        else
            config.IsSplitterDraggingX = false;

        if (active)
        {
            float mouseX   = ImGui::GetIO().MousePos.x;
            float maxSize0 = totalWidth - min_size1;

            *size0 = ImClamp(mouseX - window->Pos.x, min_size0, maxSize0);
            if (size1)
                *size1 = totalWidth - *size0;
        }
    }
    else
    {
        // For a bottom-anchored panel, the drag bar sits at the top edge of the window
        float y = winPos.y; // top edge of the bottom panel

        ImRect bb(
          ImVec2(winPos.x,             y - thickness * 0.5f),
          ImVec2(winPos.x + winSize.x, y + thickness * 0.5f)
       );

        ImGui::SetCursorPos(ImVec2(0.0f, -(hit * 0.5f)));
        ImGui::InvisibleButton("##SplitterH", ImVec2(winSize.x, thickness + hit));

        bool hovered = ImGui::IsItemHovered();
        bool active  = ImGui::IsItemActive();

        ImU32 barColor;
        if (active)
           barColor = IM_COL32(0, 120, 255, 255);
        else if (hovered)
           barColor = IM_COL32(255, 255, 255, 255);
        else
           barColor = ImGui::GetColorU32(ImGuiCol_Separator);

        window->DrawList->AddRectFilled(
          bb.Min, bb.Max,
          barColor
       );

        if (hovered || active)
           config.IsSplitterDraggingY = true;
        else
           config.IsSplitterDraggingY = false;

        if (active)
        {
           float mouseY = ImGui::GetIO().MousePos.y;
           // totalWidth here is vp->WorkSize.y, compute height from bottom
           float newHeight = totalWidth - (mouseY - ImGui::GetMainViewport()->WorkPos.y);
           *size0 = ImClamp(newHeight, min_size0, totalWidth - min_size1);
        }
    }

    ImGui::PopID();
}


void UIPanel::Render(UIData& data, GLFWwindow* glfwWindow, Window* window, Scene* scene, Input* input, entt::entity selected)
{

	const ImGuiViewport* vp = ImGui::GetMainViewport();

	static bool  s_FullscreenViewport = false;
	static float savedUiWidth = -1.0f;
	static float savedBottomHeight = -1.0f;

	static float uiWidth = 0.0f;
	if (uiWidth <= 0.0f)
		uiWidth = vp->WorkSize.x * 0.30f;
	else if (s_UiWidth > 0.0f)
		uiWidth = s_UiWidth;

	static float bottomHeight = 250.0f;
	if (s_BottomHeight > 0.0f)
		bottomHeight = s_BottomHeight;

	float splitterThickness = 1.5f;
	float dummySize = splitterThickness;

	float viewportX, viewportY, viewportW, viewportH;
	const float hitSize = 16.0f;
	const float triSize = 8.0f;


	if (!s_FullscreenViewport)
	{
		ImGui::SetNextWindowPos( ImVec2(vp->WorkPos.x, vp->WorkPos.y) );
		ImGui::SetNextWindowSize(ImVec2(uiWidth + 3.0f, vp->WorkSize.y));
		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse;


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.0f)); // fully opaque

		ImGui::Begin("UIPanel", nullptr, flags);

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();

		float minPanelSize = 30.0f;
		Splitter(data, s_Config, glfwWindow, true, splitterThickness, &uiWidth, &dummySize, minPanelSize,
			splitterThickness, vp->WorkSize.x);
		s_UiWidth = uiWidth;
		ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::BeginChild("UIPanelContent", ImVec2(uiWidth - splitterThickness, 0), false);
		ImGui::PopStyleVar();

		{ // Begin Left Panel ===========================================================================
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

				ImGui::Text("Renderer (X, Y): (%d, %d)", window->GetRenderX(), window->GetRenderY());

				if (ImGui::Button("Show Crosshair")) data.showCrosshair = !data.showCrosshair;
			}

			if (ImGui::CollapsingHeader("Scene Config"))
			{
				ImGui::PushItemWidth(70);

				ImGui::PopItemWidth();
				if (ImGui::Button("Show Grid"))          data.showGrid     = !data.showGrid;
				if (ImGui::Button("Show Grid Axes"))     data.showAxes	   = !data.showAxes;
				if (ImGui::Button("Show Skybox"))		 data.showSkybox   = !data.showSkybox;
				if (!data.showSkybox)
					ImGui::ColorEdit4("Base Color", &data.baseSkyColor.x);

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
							// Try to match the current material to a preset
							materialIdx = (int)FindMatchingMaterialPreset(matPtr->Desc);
						else
							materialIdx = (int)MATERIALS::None;

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
						float entityPitch  = glm::degrees(tc.Rotation.x);
						float entityYaw = glm::degrees(tc.Rotation.y);
						float entityRoll = glm::degrees(tc.Rotation.z);

						// Keep values in a nice UI range
						if (entityPitch  > 180.0f) entityPitch  -= 360.0f;
						if (entityYaw > 180.0f) entityYaw -= 360.0f;
						if (entityRoll > 180.0f) entityRoll -= 360.0f;

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

						if (ImGui::SliderFloat("Entity Yaw", &entityYaw, -180.0f, 180.0f))
						{
							glm::vec3 r = tc.Rotation;
							r.y = glm::radians(entityYaw);
							tc.SetRotation(r);
						}

						if (ImGui::SliderFloat("Entity Pitch", &entityPitch, -180.0f, 180.0f))
						{
							glm::vec3 r = tc.Rotation;
							r.x = glm::radians(entityPitch);
							tc.SetRotation(r);
						}

						if (ImGui::SliderFloat("Entity Roll", &entityRoll, -180.0f, 180.0f))
						{
							glm::vec3 r = tc.Rotation;
							r.z = glm::radians(entityRoll);
							tc.SetRotation(r);
						}

						if (ImGui::Button("Reset Entity Rotation"))
						{
							glm::vec3 r = tc.Rotation;
							r.x = glm::radians(0.0);
							r.y = glm::radians(0.0);
							r.z = glm::radians(0.0);
							tc.SetRotation(r);
						}


						if (ImGui::Button("Show Wireframe"))
						{
							if (e.HasComponent<WireframeComponent>())
								e.RemoveComponent<WireframeComponent>();
							else
								e.AddComponent<WireframeComponent>();
						}

						ImGui::Dummy(ImVec2(0.0f, 8.0f)); ImGui::Separator(); ImGui::Dummy(ImVec2(0.0f, 8.0f));

						if  (( e.HasComponent<MeshComponent>() ) &&
							 !e.HasComponent<MaterialComponent>() &&
						  	 !e.HasComponent<LightComponent>())
						{
							MeshComponent* mc = nullptr;

							if (e.HasComponent<MeshComponent>())
							{
								mc = &e.GetComponent<MeshComponent>();
							}
							else if (e.HasComponent<ModelRootComponent>())
							{
								auto& root = e.GetComponent<ModelRootComponent>();
								if (!root.Parts.empty())
								{
									Entity part = scene->GetEntityByID(root.Parts[0]);
									if (part && part.HasComponent<MeshComponent>())
										mc = &part.GetComponent<MeshComponent>();
								}
							}


							ImGui::ColorEdit4("Base Color", &mc->BaseColor.x);
							if (ImGui::Checkbox("Use Normal Map Colors", &mc->UseNormalColors)) {}

							if (mc && ImGui::Button("Add Material Component"))
							{
								auto& matc = e.AddComponent<MaterialComponent>();
								if (mc->Material)
								{
									matc.BaseMaterial = mc->Material->Handle;
									matc.Desc = mc->Material->Desc;
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

							// Don't allow models to remove material component
							if (!e.HasComponent<ModelRootComponent>() && ImGui::Button("Remove Material Component"))
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

		} // End Left Panel ==============================================================================

		ImGui::EndChild();
		ImGui::End(); // Left Panel


		// First, just set position/size and open the window
		ImGui::SetNextWindowPos(ImVec2(
			vp->WorkPos.x + uiWidth,
			vp->WorkPos.y + vp->WorkSize.y - bottomHeight
		));
		ImGui::SetNextWindowSize(ImVec2(
			vp->WorkSize.x - uiWidth,
			bottomHeight
		));

		ImGuiWindowFlags bottomFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.0f));

		ImGui::Begin("BottomPanel", nullptr, bottomFlags);

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();

		Splitter(
			data,
			s_Config,
			glfwWindow,
			false,
			splitterThickness,
			&bottomHeight,
			nullptr,
			minPanelSize,
			splitterThickness,
			vp->WorkSize.y
		);

		s_BottomHeight = bottomHeight;

		ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));

		ImGui::End(); // Bottom Panel

		viewportX = vp->WorkPos.x + uiWidth;
		viewportY = vp->WorkPos.y;
		viewportW = vp->WorkSize.x - uiWidth;
		viewportH = vp->WorkSize.y - bottomHeight;

		ImGui::SetNextWindowPos(ImVec2(
			viewportX + 6.0f,
			viewportY + viewportH - 24.0f
		));

		ImGui::Begin("ViewportCornerOverlay", nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoBackground
		);

		ImVec2 winPos = ImGui::GetWindowPos();

		ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
		ImGui::InvisibleButton("##CornerDragHit", ImVec2(hitSize, hitSize));
		s_Config.IsCornerDragging = ImGui::IsItemActive();

		s_Config.IsCornerHovered = ImGui::IsItemHovered();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		dl->AddTriangleFilled(
			ImVec2(winPos.x,           winPos.y + hitSize),      // bottom-left
			ImVec2(winPos.x + hitSize, winPos.y + hitSize),      // bottom-right
			ImVec2(winPos.x,           winPos.y),                // top-left
			IM_COL32(200,200,200,255)
		);

		ImGui::End(); // Bottom Panel

		if (s_Config.IsCornerDragging)
		{
			float aspect = scene->GetPrimaryCameraEntity()
				.GetComponent<CameraComponent>().Camera.GetAspect();

			float renderW = vp->WorkSize.x - uiWidth;
			float renderH = vp->WorkSize.y - bottomHeight;

			ImVec2 delta = ImGui::GetIO().MouseDelta;
			renderW -= delta.x;
			renderW = std::max(renderW, 100.0f);

			renderH = renderW / aspect;
			renderH = std::max(renderH, 80.0f);

			uiWidth      = vp->WorkSize.x - renderW;
			bottomHeight = vp->WorkSize.y - renderH;

			uiWidth      = std::max(uiWidth, minPanelSize);
			bottomHeight = std::max(bottomHeight, minPanelSize);

		}

		s_UiWidth      = uiWidth;
		s_BottomHeight = bottomHeight;
	}
	else
	{
		uiWidth = 0.0f;
		bottomHeight = 0.0f;

		viewportX = vp->WorkPos.x;
		viewportY = vp->WorkPos.y;
		viewportW = vp->WorkSize.x;
		viewportH = vp->WorkSize.y;

		s_UiWidth      = uiWidth;
		s_BottomHeight = bottomHeight;

		s_Config.IsSplitterDraggingX = false;
		s_Config.IsSplitterDraggingY = false;
		s_Config.IsCornerDragging    = false;
		s_Config.IsCornerHovered     = false;
	}

	const float btnW = 80.0f;
	const float btnH = 80.0f;
	const float inset   = 5.0f;
	ImGui::SetNextWindowPos(ImVec2(
		viewportX + viewportW - btnW - inset,
		viewportY + viewportH - (btnH / 3) - inset
	));

	ImGui::SetNextWindowSize(ImVec2(btnW, btnH));

	ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

	if (ImGui::Begin("ViewportFullscreenButton",
		nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::SetCursorPos(ImVec2(0, 0));


		ImVec2 size(btnW, 0.0f);
		if (ImGui::Button("Fullscreen", size))
		{
			if (!s_FullscreenViewport)
			{
				savedUiWidth = uiWidth;
				savedBottomHeight = bottomHeight;
			}
			else
			{
				if (savedUiWidth > 0.0f) uiWidth = savedUiWidth;
				if (savedBottomHeight > 0.0f) bottomHeight = savedBottomHeight;
			}

			s_FullscreenViewport = !s_FullscreenViewport;
		}
		ImGui::End();
	}

	SetCursors(glfwWindow);
}