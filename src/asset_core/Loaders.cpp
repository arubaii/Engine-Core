#include <stdexcept>
#include <yaml-cpp/yaml.h>
#include "stb/stb_image.h" // Need
#include "AssetTypes.h"
#include "Loaders.h"
#include "AssetManager.h"
#include "Serializers.h"
#include "asset_io/MeshImporter.h"
#include "asset_io/ModelImporter.h"
#include "audio/AudioAPI.h"
#include "audio/AudioDecoder.h"
#include "renderer_core/Shader.h"
#include "renderer_core/Mesh.h"
#include "renderer_core/GLTexture2D.h"


Ref<Asset> AudioClipLoader::LoadAsset(const AssetMetaData& meta)
{
	// Decode entire audio file into PCM
	AudioDecodeResult decoded = AudioDecoder::Decode(meta.FilePath);

	auto clip = CreateRef<AudioClipAsset>();
	clip->Handle     = meta.Handle;
	clip->Type       = meta.Type;
	clip->Channels   = decoded.Channels;
	clip->SampleRate = decoded.SampleRate;
	clip->Duration   = decoded.Duration;

	// Upload to audio backend
	clip->BufferID = AudioAPI::CreateBuffer(
		decoded.Samples,
		decoded.SampleCount,
		decoded.Channels,
		decoded.SampleRate
	);

	return clip;
}

Ref<Asset> AudioStreamLoader::LoadAsset(const AssetMetaData& meta)
{
	auto stream = CreateRef<AudioStreamAsset>();
	stream->Handle = meta.Handle;
	stream->Type   = meta.Type;

	// Create decoder only (no full decode)
	stream->StreamHandle = AudioDecoder::CreateStream(meta.FilePath);

	AudioStreamInfo info = AudioDecoder::GetStreamInfo(stream->StreamHandle);
	stream->Channels   = info.Channels;
	stream->SampleRate = info.SampleRate;
	stream->Duration   = info.Duration;

	return stream;
}

Ref<Asset> ShaderLoader::LoadAsset(const AssetMetaData& meta)
{
	auto shaderAsset = CreateRef<ShaderAsset>();
	shaderAsset->Handle = meta.Handle;
	shaderAsset->Type   = meta.Type;

	ShaderDesc paths = ShaderSerializer::Deserialize(meta.FilePath);
	shaderAsset->VertexSource   = paths.VertexPath;
	shaderAsset->FragmentSource = paths.FragmentPath;

	// Compile once here
	shaderAsset->Compiled = Shader::Create(
		shaderAsset->VertexSource,
		shaderAsset->FragmentSource
	);

	return shaderAsset;
}

Ref<Asset> MeshLoader::LoadAsset(const AssetMetaData& meta)
{
	MeshData imported = MeshImporter::Import(meta.FilePath);

	auto mesh = CreateRef<MeshAsset>();
	mesh->Handle = meta.Handle;
	mesh->Type   = meta.Type;

	mesh->MeshData = CreateRef<Mesh>();

	const size_t vertexCount = imported.Vertices.size() / 3;
	mesh->MeshData->Vertices.resize(vertexCount);

	for (size_t i = 0; i < vertexCount; i++)
	{
		Vertex v{};

		// Position
		v.Position = {
			imported.Vertices[i * 3 + 0],
			imported.Vertices[i * 3 + 1],
			imported.Vertices[i * 3 + 2]
		};

		// Normal
		if (imported.Normals.size() >= (i + 1) * 3)
		{
			v.Normal = {
				imported.Normals[i * 3 + 0],
				imported.Normals[i * 3 + 1],
				imported.Normals[i * 3 + 2]
			};
		}
		else
			v.Normal = { 0.0f, 0.0f, 1.0f };

		// Texcoord
		if (imported.TexCoords.size() >= (i + 1) * 2)
		{
			v.TexCoord = {
				imported.TexCoords[i * 2 + 0],
				imported.TexCoords[i * 2 + 1]
			};
		}
		else
			v.TexCoord = { 0.0f, 0.0f };

		// Color
		if (imported.Colors.size() >= (i + 1) * 3)
		{
			v.Color = {
				imported.Colors[i * 3 + 0],
				imported.Colors[i * 3 + 1],
				imported.Colors[i * 3 + 2]
			};
		}
		else
			v.Color = glm::vec3(1.0f);


		// Tangent
		if (imported.Tangents.size() >= (i + 1) * 3)
		{
			v.Tangent = {
				imported.Tangents[i * 3 + 0],
				imported.Tangents[i * 3 + 1],
				imported.Tangents[i * 3 + 2]
			};
		}
		else
		{
			v.Tangent = { 1.0f, 0.0f, 0.0f };
		}

		// Bitangent
		if (imported.Bitangents.size() >= (i + 1) * 3)
		{
			v.Bitangent = {
				imported.Bitangents[i * 3 + 0],
				imported.Bitangents[i * 3 + 1],
				imported.Bitangents[i * 3 + 2]
			};
		}
		else
		{
			v.Bitangent = { 0.0f, 1.0f, 0.0f };
		}

		mesh->MeshData->Vertices[i] = v;
	}

	// Indices
	mesh->MeshData->Indices = std::move(imported.Indices);
	mesh->IndexCount        = (uint32_t)mesh->MeshData->Indices.size();

	return mesh;
}

Ref<Asset> MaterialLoader::LoadAsset(const AssetMetaData& meta)
{
	MaterialDesc desc = MaterialSerializer::Deserialize(meta.FilePath);

	Ref<MaterialAsset> mat = CreateRef<MaterialAsset>();
	mat->Handle = meta.Handle;
	mat->Type   = meta.Type;
	mat->Desc   = desc;

	return mat;
}

Ref<Asset> ModelLoader::LoadAsset(const AssetMetaData& meta)
{
	YAML::Node root = YAML::LoadFile(meta.FilePath.string());
	auto modelNode = root["Model"];
	auto submeshesNode = modelNode["Submeshes"];

	auto model = CreateRef<ModelAsset>();
	model->Handle = meta.Handle;
	model->Type   = meta.Type;

	for (auto sm : submeshesNode)
	{
		ModelAsset::Submesh sub;

		sub.Mesh        = AssetManager::GetAsset<MeshAsset>( (AssetHandle)sm["Mesh"].as<uint64_t>() );
		sub.Material    = AssetManager::GetAsset<MaterialAsset>( (AssetHandle)sm["Material"].as<uint64_t>() );
		sub.IndexOffset = sm["IndexOffset"].as<uint32_t>();
		sub.IndexCount  = sm["IndexCount"].as<uint32_t>();

		model->Submeshes.push_back(sub);
	}

	return model;
}

Ref<Asset> TextureLoader::LoadAsset(const AssetMetaData& meta)
{
	int width, height, channels;

	stbi_info(meta.FilePath.string().c_str(), &width, &height, &channels);

	auto tex = CreateRef<TextureAsset>();
	tex->Handle = meta.Handle;
	tex->Type   = meta.Type;
	tex->Width  = width;
	tex->Height = height;

	// GPU Upload
	tex->Texture = new GLTexture2D(meta.FilePath.string());

	return tex;
}
