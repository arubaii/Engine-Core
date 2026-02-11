#pragma once
#include <glm/vec3.hpp>
#include "Asset.h"
#include "Serializers.h"
#include "renderer_core/GLTexture2D.h"
#include "utils/SmartPtrs.h"
#include "renderer_core/Shader.h"
#include "renderer_core/Mesh.h"


static AssetType DeduceAssetType(const std::filesystem::path& path)
{
	auto ext = path.extension().string();

	// textures
	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".hdr")
		return AssetType::Texture;

	// shaders
	if (ext == ".vert" || ext == ".frag")
		return AssetType::Shader;

	// engine assets
	if (ext == ".mat")
		return AssetType::Material;
	if (ext == ".mesh")
		return AssetType::Mesh;
	if (ext == ".model")
		return AssetType::Model;

	return AssetType::None;
}

static AssetType AssetTypeFromExtension(const std::string& ext)
{
	if (ext == ".model") return AssetType::Model;
	if (ext == ".mesh")  return AssetType::Mesh;
	if (ext == ".mat")   return AssetType::Material;

	if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".hdr")
		return AssetType::Texture;

	if (ext == ".vert" || ext == ".frag")
		return AssetType::Shader;

	return AssetType::None;
}


// <30 seconds
struct AudioClipAsset : public Asset
{
	uint32_t BufferID = 0;
	uint32_t Channels = 0;
	uint32_t SampleRate = 0;
	float Duration = 0.0f;
};

// >=30 seconds
struct AudioStreamAsset : public Asset
{
	// Opaque streaming source (decoder, handle, etc.)
	void* StreamHandle = nullptr;

	uint32_t Channels = 0;
	uint32_t SampleRate = 0;
	float Duration = 0.0f;
};

struct ShaderAsset : public Asset
{
	std::string VertexSource;
	std::string FragmentSource;

	Ref<Shader> Compiled;
};


struct MaterialAsset : public Asset
{
	MaterialDesc Desc;
};

struct MeshAsset : public Asset
{
	Ref<Mesh> MeshData;
	uint32_t IndexCount = 0;
};

struct ModelAsset : public Asset
{
	struct Submesh
	{
		Ref<MeshAsset>     Mesh;
		Ref<MaterialAsset> Material;
		uint32_t      IndexOffset = 0;
		uint32_t      IndexCount  = 0;
	};

	std::vector<Submesh> Submeshes;
};


struct TextureAsset : public Asset
{
	GLTexture2D* Texture = nullptr;
	uint32_t Width = 0, Height = 0;
};




