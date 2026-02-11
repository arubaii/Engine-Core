#pragma once
#include <string>
#include <cstdint>

class GLHDRTexture2D
{
public:
	GLHDRTexture2D(const std::string& path, bool clampToEdge);
	~GLHDRTexture2D();

	void Bind(uint32_t slot) const;
	uint32_t GetID() const { return m_ID; }

private:
	uint32_t m_ID = 0;
};