#include "Skybox.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>

Skybox::Skybox(Ref<Shader> s)
	: m_Shader(std::move(s))
{

	static const float cubeVerts[] = {
		// back
		-1,  1, -1,  -1, -1, -1,   1, -1, -1,
		 1, -1, -1,   1,  1, -1,  -1,  1, -1,
		// left
		-1, -1,  1,  -1, -1, -1,  -1,  1, -1,
		-1,  1, -1,  -1,  1,  1,  -1, -1,  1,
		// right
		 1, -1, -1,   1, -1,  1,   1,  1,  1,
		 1,  1,  1,   1,  1, -1,   1, -1, -1,
		// front
		-1, -1,  1,  -1,  1,  1,   1,  1,  1,
		 1,  1,  1,   1, -1,  1,  -1, -1,  1,
		// top
		-1,  1, -1,   1,  1, -1,   1,  1,  1,
		 1,  1,  1,  -1,  1,  1,  -1,  1, -1,
		// bottom
		-1, -1, -1,  -1, -1,  1,   1, -1, -1,
		 1, -1, -1,  -1, -1,  1,   1, -1,  1
	};

	m_VAO.Bind();
	m_VBO = VertexBuffer(sizeof(cubeVerts), cubeVerts);

	VertexBufferLayout layout;
	layout.Push<float>(3); // aPos
	m_VAO.AddBuffer(m_VBO, layout);

	m_VAO.ResetAttribIndex();
}

