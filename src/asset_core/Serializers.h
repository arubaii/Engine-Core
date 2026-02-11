#pragma once
#include <filesystem>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Asset.h"
#include "AssetSerializer.h"


struct MaterialDesc // Short for Descriptor
{
	AssetHandle ShaderProgram{0};

	// Texture handles
	AssetHandle BaseColorTexture{0};
	AssetHandle NormalTexture{0};
	AssetHandle MetallicRoughnessTexture{0};
	AssetHandle OcclusionTexture{0};
	AssetHandle EmissiveTexture{0};

	// Scalar parameters
	glm::vec4 BaseColorFactor = glm::vec4(1.0f);
	float MetallicFactor  = 1.0f;
	float RoughnessFactor = 1.0f;
	float LightBoostFactor = 1.0f;

	glm::vec3 EmissiveFactor = glm::vec3(0.0f);
	float EmissiveStrength = 1.0f;

	// GLTF config
	std::string AlphaMode = "OPAQUE";  // MASK / BLEND
	float AlphaCutoff = 0.5f;
	bool  DoubleSided = false;
};

class MaterialSerializer : public AssetSerializer
{
public:
	static MaterialDesc Deserialize(const std::filesystem::path& path);
	static void Serialize(const std::filesystem::path& path, const MaterialDesc& material);
};

struct ShaderDesc
{
	std::filesystem::path VertexPath;
	std::filesystem::path FragmentPath;
};


class ShaderSerializer : public AssetSerializer
{
public:
	static ShaderDesc Deserialize(const std::filesystem::path& path);
};


