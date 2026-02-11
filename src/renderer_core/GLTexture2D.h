#pragma once
#include <string>
#include "GLcommon.h"

class GLTexture2D
{
public:
	GLTexture2D(const std::string& path);
	~GLTexture2D();

	void Bind(uint32_t slot = 0) const;

	inline uint32_t GetID() const { return m_ID; }

private:
	uint32_t m_ID = 0;
};
