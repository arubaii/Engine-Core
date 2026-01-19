#pragma once
#include "utils/SmartPtrs.h"
#include "renderer_core/VertexArray.h"
#include "renderer_core/IndexBuffer.h"
#include "renderer_core/Mesh.h"
#include "renderer_core/Shader.h"


class Renderer
{
public:
	void SetShader(const Ref<Shader>& shader)
    {
         m_Shader = shader;
    }

    void SetMVP(const glm::mat4& mvp)
    {
        assert(m_Shader);
        m_Shader->SetMat4("u_MVP", mvp);
    }
	void Clear() const;
	void Draw(const VertexArray& va, const IndexBuffer& ib) const;
	void DrawLines(const VertexArray &va, const IndexBuffer &ib) const;

	void DrawOutline(const Ref<Shader>& outlineShader, const glm::mat4& MVP, const VertexArray& va, const IndexBuffer& ib);

private:
	Ref<Shader> m_Shader;
};