#pragma once
#include <utility>
#include "AssetMetaData.h"
#include "AssetLoader.h"


class AssetManager
{
private:
	static std::unordered_map<AssetHandle, AssetMetaData>	   s_Metadata;
	static std::unordered_map<AssetHandle, Ref<Asset>>		   s_LoadedAssets;
	static std::unordered_map<AssetType, Scope<AssetLoader>>   s_Serializers;

public:
	static void Init();

	static AssetHandle ImportAsset(const std::filesystem::path& path);

	template<typename T>
	static Ref<T> GetAsset(AssetHandle handle);
	static Ref<Asset> LoadAssetInternal(const AssetMetaData& meta);
	static void UnloadAsset(AssetHandle handle);

	static bool IsLoaded(AssetHandle handle);
	static bool Exists(AssetHandle handle);

	static void LoadRegistry(const std::filesystem::path& path);

	static void SaveRegistry(const std::string &path);

	static AssetHandle GetHandleForPath(const std::filesystem::path& path);

};

template<typename T>
Ref<T> AssetManager::GetAsset(AssetHandle handle)
{
	static_assert(std::is_base_of_v<Asset, T>);

	// Guard invalid handle
	if (handle == 0)
		return nullptr;

	// Already loaded?
	auto it = s_LoadedAssets.find(handle);
	if (it != s_LoadedAssets.end())
		return std::static_pointer_cast<T>(it->second);

	// Guard missing metadata (prevents unordered_map::at crash)
	auto metaIt = s_Metadata.find(handle);
	if (metaIt == s_Metadata.end())
		return nullptr;

	const auto& meta = metaIt->second;
	Ref<Asset> asset = LoadAssetInternal(meta);

	s_LoadedAssets[handle] = asset;
	return std::static_pointer_cast<T>(asset);
}
