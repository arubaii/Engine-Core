#include "GLMesh.h"
#include "GLcommon.h"
#include <cstddef>

GLMesh::GLMesh() {}

GLMesh::~GLMesh()
{
	if (m_EBO) glDeleteBuffers(1, &m_EBO);
	if (m_VBO) glDeleteBuffers(1, &m_VBO);
	if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void GLMesh::Upload(const Mesh& mesh)
{
	m_IndexCount = static_cast<uint32_t>(mesh.Indices.size());
	if (m_IndexCount == 0)
		return;

	if (!m_VAO)
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
	}

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		mesh.Vertices.size() * sizeof(Vertex),
		mesh.Vertices.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		mesh.Indices.size() * sizeof(uint32_t),
		mesh.Indices.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, Position)
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, Normal)
	);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, TexCoord)
	);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, Color)
	);

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(
		4, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, Tangent)
	);

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(
		5, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		(void*)offsetof(Vertex, Bitangent)
	);

	glBindVertexArray(0);
}

void GLMesh::Draw() const
{
	if (!m_VAO || m_IndexCount == 0)
		return;

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
}