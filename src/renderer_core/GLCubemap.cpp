#include "GLCubemap.h"
#include "stb/stb_image.h"
#include <cstdio>

bool GLCubemap::LoadFromFiles(const std::array<std::string, 6>& faces)
{
	if (m_ID == 0)
		glGenTextures(1, &m_ID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

	stbi_set_flip_vertically_on_load(false); // Cubemap faces are usually created unflipped

	for (int i = 0; i < 6; ++i)
	{
		int w = 0, h = 0, channels = 0;
		unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &channels, 0);
		if (!data)
		{
			std::fprintf(stderr, "[Cubemap] Failed to load: %s\n", faces[i].c_str());
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			return false;
		}

		GLenum fmt = (channels == 4) ? GL_RGBA : GL_RGB;

		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			fmt,
			w,
			h,
			0,
			fmt,
			GL_UNSIGNED_BYTE,
			data
		);

		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return true;
}