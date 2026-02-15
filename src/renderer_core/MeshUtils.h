#pragma once
#include "Mesh.h"
#include "asset_io/MeshImporter.h"





namespace MeshUtils
{
	inline float CalculateMeshRadius(const Mesh& mesh, glm::vec3& outCenter)
	{
		if (mesh.Vertices.empty())
			return 0.0f;

		// First pass: find AABB center
		glm::vec3 minBounds = mesh.Vertices[0].Position;
		glm::vec3 maxBounds = mesh.Vertices[0].Position;

		for (const auto& vertex : mesh.Vertices)
		{
			minBounds = glm::min(minBounds, vertex.Position);
			maxBounds = glm::max(maxBounds, vertex.Position);
		}

		outCenter = (minBounds + maxBounds) * 0.5f;

		// Second pass: find max distance from center
		float maxRadius = 0.0f;
		for (const auto& vertex : mesh.Vertices)
		{
			float distance = glm::length(vertex.Position - outCenter);
			maxRadius = glm::max(maxRadius, distance);
		}

		return maxRadius;
	}

	static void GenerateSphericalUVs(MeshData& data)
	{
		const size_t count = data.Vertices.size() / 3;

		for (size_t i = 0; i < count; i++)
		{
			glm::vec3 p(
				data.Vertices[i*3 + 0],
				data.Vertices[i*3 + 1],
				data.Vertices[i*3 + 2]
			);

			float u = 0.5f + atan2(p.z, p.x) / (2.0f * M_PI);
			float v = 0.5f - asin(p.y / glm::length(p)) / M_PI;

			data.TexCoords[i*2 + 0] = u;
			data.TexCoords[i*2 + 1] = v;
		}
	}

	static void GenerateTangents(MeshData& data)
	{
	    size_t vertexCount = data.Vertices.size() / 3;
	    size_t indexCount = data.Indices.size();

	    data.Tangents.resize(vertexCount * 3);
	    data.Bitangents.resize(vertexCount * 3);

	    std::vector<glm::vec3> tan1(vertexCount, glm::vec3(0));
	    std::vector<glm::vec3> tan2(vertexCount, glm::vec3(0));

	    for (size_t i = 0; i < indexCount; i += 3)
	    {
	        uint32_t i1 = data.Indices[i + 0];
	        uint32_t i2 = data.Indices[i + 1];
	        uint32_t i3 = data.Indices[i + 2];

	        glm::vec3 v1 = glm::vec3(
	            data.Vertices[i1*3+0],
	            data.Vertices[i1*3+1],
	            data.Vertices[i1*3+2]
	        );
	        glm::vec3 v2 = glm::vec3(
	            data.Vertices[i2*3+0],
	            data.Vertices[i2*3+1],
	            data.Vertices[i2*3+2]
	        );
	        glm::vec3 v3 = glm::vec3(
	            data.Vertices[i3*3+0],
	            data.Vertices[i3*3+1],
	            data.Vertices[i3*3+2]
	        );

	        glm::vec2 w1 = glm::vec2(
	            data.TexCoords[i1*2+0],
	            data.TexCoords[i1*2+1]
	        );
	        glm::vec2 w2 = glm::vec2(
	            data.TexCoords[i2*2+0],
	            data.TexCoords[i2*2+1]
	        );
	        glm::vec2 w3 = glm::vec2(
	            data.TexCoords[i3*2+0],
	            data.TexCoords[i3*2+1]
	        );

	        glm::vec3 e1 = v2 - v1;
	        glm::vec3 e2 = v3 - v1;

	        glm::vec2 d1 = w2 - w1;
	        glm::vec2 d2 = w3 - w1;

	        float r = (d1.x * d2.y - d1.y * d2.x);
	        if (r == 0.0f) r = 1.0f;
	        r = 1.0f / r;

	        glm::vec3 sdir = (e1 * d2.y - e2 * d1.y) * r;
	        glm::vec3 tdir = (e2 * d1.x - e1 * d2.x) * r;

	        tan1[i1] += sdir;
	        tan1[i2] += sdir;
	        tan1[i3] += sdir;

	        tan2[i1] += tdir;
	        tan2[i2] += tdir;
	        tan2[i3] += tdir;
	    }

	    for (size_t i = 0; i < vertexCount; i++)
	    {
	        glm::vec3 n = glm::vec3(
	            data.Normals[i*3+0],
	            data.Normals[i*3+1],
	            data.Normals[i*3+2]
	        );

	        glm::vec3 t = tan1[i];

	        t = glm::normalize(t - n * glm::dot(n, t));

	        glm::vec3 b = glm::cross(n, t);
	        if (glm::dot(b, tan2[i]) < 0.0f)
	            t = -t;

	        data.Tangents[i*3+0] = t.x;
	        data.Tangents[i*3+1] = t.y;
	        data.Tangents[i*3+2] = t.z;
	    }
	}

	static std::vector<Vertex> BuildVertexBuffer(const MeshData& data)
	{
		size_t count = data.Vertices.size() / 3;
		std::vector<Vertex> out(count);

		for (size_t i = 0; i < count; i++)
		{
			out[i].Position = glm::vec3(
				data.Vertices[i*3+0],
				data.Vertices[i*3+1],
				data.Vertices[i*3+2]
			);

			out[i].Normal = glm::vec3(
				data.Normals[i*3+0],
				data.Normals[i*3+1],
				data.Normals[i*3+2]
			);

			out[i].TexCoord = glm::vec2(
				data.TexCoords[i*2+0],
				data.TexCoords[i*2+1]
			);

			out[i].Color = glm::vec3(
				data.Colors[i*3+0],
				data.Colors[i*3+1],
				data.Colors[i*3+2]
			);

			out[i].Tangent = glm::vec3(
				data.Tangents[i*3+0],
				data.Tangents[i*3+1],
				data.Tangents[i*3+2]
			);

			out[i].Bitangent = glm::vec3(
				data.Bitangents[i*3+0],
				data.Bitangents[i*3+1],
				data.Bitangents[i*3+2]
			);
		}

		return out;
	}





} // END NAMESPACE =================================================================