#include "utils/SmartPtrs.h"
#include "Application.h"

#include "asset_core/AssetManager.h"
#include "utils/Log.h"
#include "utils/LoadingSplash.h"
#include "../scene_core/ecs/Entity.h"


Application::Application(const ApplicationProperties& props)
	: m_AppProps(props)
{

	m_Window = Window::Create
	(
		m_AppProps.WindowProps.Width,
		m_AppProps.WindowProps.Height,
		m_AppProps.WindowProps.Title,
		m_AppProps.WindowProps.MonitorSelected
	);

	{
		LoadingSplash splash;
		splash.Init();
		splash.Draw(*m_Window, "Loading...", 15);
		m_Window->SwapBuffers();
		m_Window->PollEvents();
		splash.Shutdown();
	}


	m_Window->AttachInput(&m_Input);
	m_Window->SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	AssetManager::Init();
	AssetManager::LoadRegistry("../assets/assets.yaml");

	m_Shader     = CreateScope<Shader>("base.vert", "base.frag");
	m_Scene      = CreateScope<Scene>(*m_Window, m_Input);
	m_Renderer   = CreateScope<Renderer>();
	m_ImGuiLayer = CreateScope<ImGuiLayer>();

	m_ImGuiLayer->OnAttach(m_Window->GetGLFWwindow());


}

Application::~Application() {}

void Application::Run()
{

	while (!m_Window->ShouldClose())
	{
		UpdateDeltaTime();
		UIUtils::UpdateLightsFlash(m_DeltaTime, m_UI.showLights);

		m_Window->PollEvents();

		m_Input.Update(m_Window->GetGLFWwindow());
		m_Scene->Update(m_DeltaTime, m_Input);
		glClearColor(m_UI.baseSkyColor.x, m_UI.baseSkyColor.y, m_UI.baseSkyColor.z, m_UI.baseSkyColor.w);
		m_Renderer->Clear();


		m_ImGuiLayer->BeginFrame();

		m_UI.fps        = static_cast<int>(1.0f / m_DeltaTime);
		m_UI.frameTime  = m_DeltaTime * 1000.0f;
		m_UI.cameraPos  = m_Scene->GetMainCameraPos();
		m_UI.pitch      = m_Scene->GetMainCameraPitch();
		m_UI.yaw        = m_Scene->GetMainCameraYaw();

		m_Scene->m_ShowGrid      		= m_UI.showGrid;
		m_Scene->m_ShowAxes      		= m_UI.showAxes;
		m_Scene->m_ShowSkybox	 		= m_UI.showSkybox;
		m_Scene->m_ShowCrosshair 		= m_UI.showCrosshair;
		m_Scene->m_ShowLights			= m_UI.showLights;
		m_Scene->m_GeometryScale		= m_UI.geometryScale;
		m_Scene->m_EnablePhysics 		= m_UI.enablePhysics;
		m_Scene->GetDragSystem().SetDragMode
		(
			m_Input,
			m_UI.dragAffectsVertical,
			m_UI.dragAffectsXZ
		);


		UIPanel::Render(m_UI,
			            m_Window->GetGLFWwindow(),
					    m_Window.get(),
		                m_Scene.get(),
		                &m_Input,
		                m_Scene->GetSelectedEntity()
		);


		// TEMP: refine later for adding more panels
		// rather than explicitly setting each panel
		float panelWidth   = UIPanel::GetPanelWidth();
		float bottomHeight = UIPanel::GetBottomHeight();
		m_Window->SetRenderRegion(panelWidth, bottomHeight);


		m_Scene->Render(*m_Renderer);



		m_ImGuiLayer->EndFrame();

		m_Input.EndFrame();
		m_Window->SwapBuffers();
	}
}


