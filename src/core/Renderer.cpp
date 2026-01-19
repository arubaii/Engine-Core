#include "Renderer.h"

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib) const
{
	va.Bind();
	ib.Bind();

	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawLines(const VertexArray& va, const IndexBuffer& ib) const
{

	va.Bind();
	ib.Bind();

	glDrawElements(GL_LINES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::DrawOutline(const Ref<Shader>& outlineShader, const glm::mat4& MVP, const VertexArray& va, const IndexBuffer& ib)
{
	glEnable(GL_POLYGON_OFFSET_LINE);

	glPolygonOffset(-1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	glLineWidth(2.0f);

	SetShader(outlineShader);
	outlineShader->Bind();
	outlineShader->SetMat4("u_MVP", MVP);
	Draw(va, ib);

	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_POLYGON_OFFSET_LINE);
}
