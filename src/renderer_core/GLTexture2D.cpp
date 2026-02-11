#include "GLTexture2D.h"
#include <stb/stb_image.h>
#include <iostream>


GLTexture2D::GLTexture2D(const std::string& path)
{
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, channels;
	stbi_set_flip_vertically_on_load(false);
	uint8_t* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

	if (!data)
	{
		std::cerr << "Failed to load texture: " << path << std::endl;
		return;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

GLTexture2D::~GLTexture2D()
{
	glDeleteTextures(1, &m_ID);
}

void GLTexture2D::Bind(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}
