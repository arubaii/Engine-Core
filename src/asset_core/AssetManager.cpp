#include <yaml-cpp/yaml.h>
#include <fstream>
#include "AssetManager.h"
#include "Loaders.h"

std::unordered_map<AssetHandle, AssetMetaData> AssetManager::s_Metadata{};
std::unordered_map<AssetHandle, Ref<Asset>>   AssetManager::s_LoadedAssets{};
std::unordered_map<AssetType, Scope<AssetLoader>> AssetManager::s_Serializers{};

void AssetManager::Init()
{

	s_Serializers[AssetType::Texture]  = CreateScope<TextureLoader>();
	s_Serializers[AssetType::Shader]   = CreateScope<ShaderLoader>();
	s_Serializers[AssetType::Material] = CreateScope<MaterialLoader>();
	s_Serializers[AssetType::Mesh]     = CreateScope<MeshLoader>();
	s_Serializers[AssetType::Model]    = CreateScope<ModelLoader>();
}


static std::filesystem::path GetAssetsRoot()
{
	// path to executable
	std::filesystem::path exe = std::filesystem::current_path();

	// Executable is inside "cmake-build-debug/"
	// Assets are one directory up, inside "../assets/"
	return exe.parent_path() / "assets";
}


// AssetManager implementation
AssetHandle AssetManager::ImportAsset(const std::filesystem::path& path)
{
	// Absolute path of the source file
	std::filesystem::path abs = std::filesystem::absolute(path);

	// Correct assets root
	std::filesystem::path assetsRoot = GetAssetsRoot();

	// Make relative to assets/
	std::filesystem::path rel = std::filesystem::relative(abs, assetsRoot).lexically_normal();

	// Already registered
	for (auto& [h, meta] : s_Metadata)
		if (meta.FilePath == rel)
			return h;

	// New asset
	AssetHandle handle = UUID();

	AssetMetaData meta;
	meta.Handle   = handle;
	meta.Type     = DeduceAssetType(path);
	meta.FilePath = rel;

	s_Metadata[handle] = meta;

	SaveRegistry((assetsRoot / "assets.yaml").string());

	return handle;
}

Ref<Asset> AssetManager::LoadAssetInternal(const AssetMetaData& meta)
{
	auto loaderIt = s_Serializers.find(meta.Type);
	if (loaderIt == s_Serializers.end())
	{
		throw std::runtime_error(
			std::string("No loader registered for type=") +
			AssetTypeToString(meta.Type) +
			" file=" + meta.FilePath.generic_string()
		);
	}

	AssetMetaData fixed = meta;
	fixed.FilePath = std::filesystem::path("../assets") / meta.FilePath;

	return loaderIt->second->LoadAsset(fixed);
}

bool AssetManager::IsLoaded(AssetHandle handle)
{
	return s_LoadedAssets.find(handle) != s_LoadedAssets.end();
}

void AssetManager::UnloadAsset(AssetHandle handle)
{
	s_LoadedAssets.erase(handle);
}

bool AssetManager::Exists(AssetHandle handle)
{
	return s_Metadata.find(handle) != s_Metadata.end();
}

void AssetManager::LoadRegistry(const std::filesystem::path& path)
{
	YAML::Node data = YAML::LoadFile(path.string());
	auto assets = data["Assets"];
	if (!assets) return;

	for (auto entry : assets)
	{
		AssetMetaData meta;
		meta.Handle = UUID{ entry["Handle"].as<uint64_t>() };
		meta.Type   = AssetTypeFromString(entry["Type"].as<std::string>());
		meta.FilePath = entry["File"].as<std::string>();
		s_Metadata[meta.Handle] = meta;
	}
}

void AssetManager::SaveRegistry(const std::string& path)
{
	YAML::Emitter out;

	out << YAML::BeginMap;
	out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

	for (auto& [handle, meta] : s_Metadata)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Handle" << YAML::Value << (uint64_t)handle;
		out << YAML::Key << "Type"   << YAML::Value << AssetTypeToString(meta.Type);
		out << YAML::Key << "File"   << YAML::Value << meta.FilePath.string();
		out << YAML::EndMap;
	}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(path);
	fout << out.c_str();
}

AssetHandle AssetManager::GetHandleForPath(const std::filesystem::path& path)
{
	std::filesystem::path rel = path.lexically_normal();
	std::string rs = rel.generic_string();

	while (rs.starts_with("../")) rs = rs.substr(3);
	while (rs.starts_with("./"))  rs = rs.substr(2);

	const char* assetsPrefix = "assets/";
	if (rs.starts_with(assetsPrefix))
		rs = rs.substr(strlen(assetsPrefix));


	for (auto& [h, meta] : s_Metadata)
	{
		std::string stored = meta.FilePath.generic_string();
		stored = std::filesystem::path(stored).lexically_normal().generic_string();

		if (stored == rs)
			return h;
	}

	return AssetHandle(0);
}
