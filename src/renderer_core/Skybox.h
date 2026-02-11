#pragma once
#include <array>
#include <string>

#include "renderer_core/Shader.h"
#include "renderer_core/VertexArray.h"
#include "renderer_core/VertexBuffer.h"
#include "renderer_core/GLCubemap.h"

class Skybox
{
public:
	Skybox() = default;
	explicit Skybox(Ref<Shader> s);

	bool LoadCubemap(const std::array<std::string, 6>& faces)
	{
		return m_Cubemap.LoadFromFiles(faces);
	}


private:
	Ref<Shader> m_Shader;
	VertexArray m_VAO;
	VertexBuffer m_VBO;
	GLCubemap m_Cubemap;
};