#pragma once
#include <string>
#include "Window.h"
#include "Renderer.h"
#include "Scene.h"
#include "io/Input.h"
#include "ui/ImGuiLayer.h"
#include "ui/UIPanel.h"
#include "renderer_core/Shader.h"

struct ApplicationProperties
{
	std::string Name = "App";
	WindowProperties WindowProps;
};

class Application
{
public:
	Application(const ApplicationProperties& props);
	~Application();

	void onResize(int width, int height);

	Window& GetWindow() { return *m_Window; }
	void Run();



	void TestTriangle();
private:
	void UpdateDeltaTime()
	{
		float currentFrame = glfwGetTime();
		m_DeltaTime = currentFrame - m_LastFrame;
		m_LastFrame = currentFrame;
	}

private:
	ApplicationProperties m_AppProps;

	Scope<Window>     m_Window;
	Scope<Renderer>   m_Renderer;
	Scope<Scene>      m_Scene;
	Scope<ImGuiLayer> m_ImGuiLayer;
	Scope<Shader>	  m_Shader;
	Input			  m_Input;
	UI		  m_UI{};


	double m_LastFrame;
	double m_DeltaTime;

	int m_WindowWidth  = 0;
	int m_WindowHeight = 0;



};


class LoadingSplash
{
public:
	void Init();
	void Shutdown();

	// draw once (call SwapBuffers() yourself after)
	void Draw(Window& win, const char* text = "Loading...", float scale = 2.0f);

private:
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_Program = 0;

	GLint m_uScreenSize = -1;
	GLint m_uColor = -1;

	std::vector<float> m_TriVerts; // x,y per vertex (pixels)
};