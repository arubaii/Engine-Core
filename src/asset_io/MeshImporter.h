#pragma once
#include <vector>
#include <filesystem>

struct MeshData
{
	std::vector<float> Vertices;
	std::vector<uint32_t> Indices;

	std::vector<float> Normals;
	std::vector<float> TexCoords;
	std::vector<float> Colors;
	std::vector<float> Tangents;
	std::vector<float> Bitangents;
};

class MeshImporter
{
public:
	static MeshData Import(const std::filesystem::path& path);
};
