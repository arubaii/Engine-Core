#include "utils/SmartPtrs.h"
#include "Application.h"

#include "asset_core/AssetManager.h"
#include "utils/Log.h"
#include "scene_core/Entity.h"
#include "stb/stb_easy_font.h"

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


	m_Window->AttachInput(m_Input);
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
		glClearColor(m_UI.greyScale, m_UI.greyScale, m_UI.greyScale, 1.0f);
		m_Renderer->Clear();
		m_Scene->Render(*m_Renderer);


		m_ImGuiLayer->BeginFrame();

		m_UI.fps        = static_cast<int>(1.0f / m_DeltaTime);
		m_UI.frameTime  = m_DeltaTime * 1000.0f;
		m_UI.cameraPos  = m_Scene->GetMainCameraPos();
		m_UI.pitch      = m_Scene->GetMainCameraPitch();
		m_UI.yaw        = m_Scene->GetMainCameraYaw();

		m_Scene->m_ShowGrid      = m_UI.showGrid;
		m_Scene->m_ShowAxes      = m_UI.showAxes;
		m_Scene->m_ShowSkybox	 = m_UI.showSkybox;
		m_Scene->m_ShowCrosshair = m_UI.showCrosshair;
		m_Scene->m_ShowLights	 = m_UI.showLights;
		m_Scene->m_GeometryScale = m_UI.geometryScale;
		m_Scene->m_DragAffectsVertical = m_UI.dragAffectsVertical;
		m_Scene->m_DragAffectsXZ = m_UI.dragAffectsXZ;


		UIPanel::Render(m_UI, m_Scene->GetSelectedEntity(), m_Scene.get());
		m_ImGuiLayer->EndFrame();

		m_Input.EndFrame();
		m_Window->SwapBuffers();
	}
}


// LoadingSplash must be able to render before the engine's asset/shader systems are initialized
// Compile/link a tiny shader program locally to avoid dependencies
static GLuint Compile(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
    }
    return s;
}

static GLuint Link(GLuint vs, GLuint fs)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
    }

    glDetachShader(p, vs);
    glDetachShader(p, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

void LoadingSplash::Init()
{

    const char* vsSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 aPosPx;
        uniform vec2 uScreenSize;
        void main()
        {
            // pixel -> NDC
            vec2 ndc = vec2(
                (aPosPx.x / uScreenSize.x) * 2.0 - 1.0,
                1.0 - (aPosPx.y / uScreenSize.y) * 2.0
            );
            gl_Position = vec4(ndc, 0.0, 1.0);
        }
    )";

    const char* fsSrc = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main()
        {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    GLuint vs = Compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = Compile(GL_FRAGMENT_SHADER, fsSrc);
    m_Program = Link(vs, fs);

    m_uScreenSize = glGetUniformLocation(m_Program, "uScreenSize");
    m_uColor      = glGetUniformLocation(m_Program, "uColor");

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void LoadingSplash::Shutdown()
{
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_Program) glDeleteProgram(m_Program);
    m_VBO = m_VAO = m_Program = 0;
}

void LoadingSplash::Draw(Window& win, const char* text, float scale)
{
    GLFWwindow* w = win.GetGLFWwindow();
    glfwMakeContextCurrent(w);

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(w, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    char buf[99999];
    int quadCount = stb_easy_font_print(0.0f, 0.0f, (char*)text, nullptr, buf, sizeof(buf));

    float textW = (float)stb_easy_font_width((char*)text);
    float textH = (float)stb_easy_font_height((char*)text);

    float originX = (fbW - textW) * 0.5f;
    float originY = (fbH - textH) * 0.5f;

	float cx = originX + textW * 0.5f;
	float cy = originY + textH * 0.5f;

    const float* v = (const float*)buf;

    m_TriVerts.clear();
    m_TriVerts.reserve((size_t)quadCount * 6 * 2);

    for (int q = 0; q < quadCount; q++)
    {
        // 4 verts
        // v0, v1, v2, v3
        const float* v0 = v + (q * 4 + 0) * 4;
        const float* v1 = v + (q * 4 + 1) * 4;
        const float* v2 = v + (q * 4 + 2) * 4;
        const float* v3 = v + (q * 4 + 3) * 4;

    	auto push2 = [&](const float* p)
    	{
    		float x = originX + p[0];
    		float y = originY + p[1];

    		// scale around text center
    		x = cx + (x - cx) * scale;
    		y = cy + (y - cy) * scale;

    		m_TriVerts.push_back(x);
    		m_TriVerts.push_back(y);
    	};

        // two triangles: (0,1,2) and (0,2,3)
        push2(v0); push2(v1); push2(v2);
        push2(v0); push2(v2); push2(v3);
    }

    glUseProgram(m_Program);
    glUniform2f(m_uScreenSize, (float)fbW, (float)fbH);
    glUniform3f(m_uColor, 0.90f, 0.90f, 0.90f);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(m_TriVerts.size() * sizeof(float)),
                 m_TriVerts.data(),
                 GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(m_TriVerts.size() / 2));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


