#pragma once
#include <array>
#include <string>

#include "renderer_core/Shader.h"
#include "renderer_core/VertexArray.h"
#include "renderer_core/VertexBuffer.h"
#include "renderer_core/GLCubemap.h"
#include <stb/stb_image.h>


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