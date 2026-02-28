#include "GLHDRTexture2D.h"
#include <stb/stb_image.h>
#include <iostream>
#include "GLcommon.h"

GLHDRTexture2D::GLHDRTexture2D(const std::string& path, bool clampToEdge)
{
	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampToEdge ? GL_CLAMP_TO_EDGE : GL_REPEAT);

	int w = 0, h = 0, n = 0;
	stbi_set_flip_vertically_on_load(true);
	float* data = stbi_loadf(path.c_str(), &w, &h, &n, 3);

	if (!data)
	{
		std::cerr << "Failed to load HDR texture: " << path << std::endl;
		glBindTexture(GL_TEXTURE_2D, 0);
		return;
	}

	std::cout << "[HDR] size=" << w << "x" << h << " channels=" << n << std::endl;

	float maxv = 0.0f;
	int count = w * h * 3;
	for (int i = 0; i < count; i++)
		maxv = std::max(maxv, data[i]);

	std::cout << "[HDR] max pixel value = " << maxv << std::endl;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, data);

	stbi_image_free(data);
}

GLHDRTexture2D::~GLHDRTexture2D()
{
	if (m_ID) glDeleteTextures(1, &m_ID);
}

void GLHDRTexture2D::Bind(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}