#pragma once
#include "utils/SmartPtrs.h"
#include "renderer_core/VertexArray.h"
#include "renderer_core/IndexBuffer.h"
#include "renderer_core/Mesh.h"
#include "renderer_core/Shader.h"


class Renderer
{
public:
	void SetShader(const Ref<Shader>& shader);
    void SetMVP(const glm::mat4& mvp);

	// Binding
	void BindVertexArray(GLuint vao) const;
	void BindCubemap(GLuint textureID, uint32_t slot = 0) const;

	// Frame control
	void Clear() const;
	void SetClearColor(const glm::vec4& color) const;

	// Depth
	void EnableDepthTest(bool enabled) const;
	void EnableDepthMask(bool enabled) const;
	void SetDepthFunc(unsigned int func) const;
	void EnableStencilTest(bool enabled) const;

	// Blending
	void EnableBlending(bool enabled) const;
	void SetBlendFunc(unsigned int src, unsigned int dst) const;

	// Face culling
	void EnableCulling(bool enabled) const;

	// Polygon mode
	void SetPolygonMode(unsigned int face, unsigned int mode) const;
	void SetPolygonOffset(float factor, float units) const;
	void EnablePolygonOffset(bool enabled) const;

	// Viewport
	void SetViewport(int x, int y, int width, int height) const;

	// Draw calls
	void Draw(const VertexArray& va, const IndexBuffer& ib) const;
	void DrawLines(const VertexArray& va, const IndexBuffer& ib) const;
	void DrawArrays(unsigned int mode, unsigned int first, unsigned int count) const;
	void DrawOutline(const glm::mat4 & mvp, const VertexArray &va, const IndexBuffer &ib);
	void DrawWireframe(const Ref<Shader>& outlineShader, const glm::vec3& color,
	                   const glm::mat4& MVP, const VertexArray& va, const IndexBuffer& ib);


	void InitOutlines(int width, int height);
	void ResizeOutlines(int width, int height);

	void BeginFrame(int frameWidth, int frameHeight, int vpX, int vpY, int vpW, int vpH);
	void BeginSelectionMask();
	void EndSelectionMask();

	void DrawSelectionMask(const Ref<Shader> &maskShader, const glm::mat4 & mvp, const VertexArray &va, const IndexBuffer &ib);
	void CompositeOutlines(const Ref<Shader> &outlineShader, const glm::vec3 & outlineColor, int thicknessPixels);

	GLuint GetSelectionFBO() const { return m_SelectionFBO; }

private:
	Ref<Shader> m_ActiveShader;

	GLuint m_SelectionFBO = 0;
	GLuint m_SelectionTex = 0;
	GLuint m_SelectionDepth = 0;
	GLuint m_FullscreenVAO = 0;

	int m_SelectionWidth = 0;
	int m_SelectionHeight = 0;

	int m_FrameWidth = 0;
	int m_FrameHeight = 0;

	int m_VpX = 0;
	int m_VpY = 0;
	int m_VpW = 0;
	int m_VpH = 0;


};