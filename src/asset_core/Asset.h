#pragma once
#include "utils/Log.h"
#include "utils/UUID.h"

enum class AssetType : uint8_t
{
	None = 0,

	Texture,
	Mesh,
	Model,
	Shader,
	Material,
	Scene,
	Audio,
	Font
};

static AssetType AssetTypeFromString(const std::string& s)
{
	if (s == "None")      return AssetType::None;
	if (s == "Texture")   return AssetType::Texture;
	if (s == "Mesh")      return AssetType::Mesh;
	if (s == "Shader")    return AssetType::Shader;
	if (s == "Material")  return AssetType::Material;
	if (s == "Model")     return AssetType::Model;

	LOG_ERROR("AssetTypeFromString: Unknown asset type: ", s);
	return AssetType::None;
}

constexpr const char* AssetTypeToString(AssetType type)
{
	switch (type)
	{
		case AssetType::None:		return "None";
		case AssetType::Texture:	return "Texture";
		case AssetType::Mesh:		return "Mesh";
		case AssetType::Model:		return "Model";
		case AssetType::Shader:		return "Shader";
		case AssetType::Material:	return "Material";
		case AssetType::Scene:		return "Scene";
		case AssetType::Audio:		return "Audio";
		case AssetType::Font:		return "Font";
	}
}


using AssetHandle = UUID;

class Asset
{
public:
	AssetHandle Handle;
	AssetType   Type = AssetType::None;
	virtual ~Asset() = default;
};