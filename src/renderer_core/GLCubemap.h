#pragma once
#include <array>
#include <string>
#include "GLcommon.h"

class GLCubemap
{
public:
	// faces in order:
	// +X, -X, +Y, -Y, +Z, -Z
	// (right, left, top, bottom, front, back)
	bool LoadFromFiles(const std::array<std::string, 6>& faces);

	void Bind(uint32_t slot = 0) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);
	}

	uint32_t GetID() const { return m_ID; }

private:
	uint32_t m_ID = 0;
};