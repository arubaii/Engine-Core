#pragma once
#include <vector>
#include "core/Window.h"

class LoadingSplash
{
public:
	void Init();
	void Shutdown();

	void Draw(Window& win, const char* text = "Loading...", float scale = 2.0f);

private:
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;
	GLuint m_Program = 0;

	GLint m_uScreenSize = -1;
	GLint m_uColor = -1;

	std::vector<float> m_TriVerts; // x,y per vertex (pixels)
};