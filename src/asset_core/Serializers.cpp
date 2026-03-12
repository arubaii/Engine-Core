#include <yaml-cpp/yaml.h>
#include "Serializers.h"
#include <fstream>
#include "YamlTypes.h"

MaterialDesc MaterialSerializer::Deserialize(const std::filesystem::path& path)
{
	YAML::Node root = YAML::LoadFile(path.string());
	YAML::Node mat  = root["Material"];

	MaterialDesc result;

	auto ReadHandle = [&](const char* key) -> AssetHandle
	{
		if (!mat[key]) return AssetHandle(0);
		return AssetHandle(mat[key].as<uint64_t>());
	};

	result.ShaderProgram            = ReadHandle("Shader");
	result.BaseColorTexture         = ReadHandle("BaseColorTexture");
	result.NormalTexture            = ReadHandle("NormalTexture");
	result.MetallicRoughnessTexture = ReadHandle("MetallicRoughnessTexture");
	result.OcclusionTexture         = ReadHandle("OcclusionTexture");
	result.EmissiveTexture          = ReadHandle("EmissiveTexture");

	if (mat["BaseColorFactor"] && mat["BaseColorFactor"].IsSequence() && mat["BaseColorFactor"].size() == 4)
	{
		auto arr = mat["BaseColorFactor"];
		result.BaseColorFactor = glm::vec4(
			arr[0].as<float>(),
			arr[1].as<float>(),
			arr[2].as<float>(),
			arr[3].as<float>()
		);
	}
	else
		result.BaseColorFactor = glm::vec4(1.0f);

	result.MetallicFactor  = mat["MetallicFactor"].as<float>(1.0f);
	result.RoughnessFactor = mat["RoughnessFactor"].as<float>(1.0f);

	if (mat["EmissiveFactor"] && mat["EmissiveFactor"].IsSequence() && mat["EmissiveFactor"].size() == 3)
	{
		auto arr = mat["EmissiveFactor"];
		result.EmissiveFactor = glm::vec3(
			arr[0].as<float>(),
			arr[1].as<float>(),
			arr[2].as<float>()
		);
	}
	else
		result.EmissiveFactor = glm::vec3(0.0f);

	result.AlphaMode   = mat["AlphaMode"].as<std::string>("OPAQUE");
	result.AlphaCutoff = mat["AlphaCutoff"].as<float>(0.5f);
	result.DoubleSided = mat["DoubleSided"].as<bool>(false);

	return result;
}
void MaterialSerializer::Serialize(const std::filesystem::path& path, const MaterialDesc& m)
{
	YAML::Emitter out;

	out << YAML::BeginMap;
	out << YAML::Key << "Material" << YAML::Value;
	out << YAML::BeginMap;

	// Handles
	out << YAML::Key << "Shader" << YAML::Value << (uint64_t)m.ShaderProgram;
	out << YAML::Key << "BaseColorTexture" << YAML::Value << (uint64_t)m.BaseColorTexture;
	out << YAML::Key << "NormalTexture" << YAML::Value << (uint64_t)m.NormalTexture;
	out << YAML::Key << "MetallicRoughnessTexture" << YAML::Value << (uint64_t)m.MetallicRoughnessTexture;
	out << YAML::Key << "OcclusionTexture" << YAML::Value << (uint64_t)m.OcclusionTexture;
	out << YAML::Key << "EmissiveTexture" << YAML::Value << (uint64_t)m.EmissiveTexture;

	// Factors
	out << YAML::Key << "BaseColorFactor" << YAML::Value
		<< YAML::Flow << YAML::BeginSeq
		<< m.BaseColorFactor.r << m.BaseColorFactor.g
		<< m.BaseColorFactor.b << m.BaseColorFactor.a
		<< YAML::EndSeq;

	out << YAML::Key << "MetallicFactor"  << YAML::Value << m.MetallicFactor;
	out << YAML::Key << "RoughnessFactor" << YAML::Value << m.RoughnessFactor;

	out << YAML::Key << "EmissiveFactor" << YAML::Value
		<< YAML::Flow << YAML::BeginSeq
		<< m.EmissiveFactor.r << m.EmissiveFactor.g << m.EmissiveFactor.b
		<< YAML::EndSeq;

	// alpha + flags
	out << YAML::Key << "AlphaMode"   << YAML::Value << m.AlphaMode;
	out << YAML::Key << "AlphaCutoff" << YAML::Value << m.AlphaCutoff;
	out << YAML::Key << "DoubleSided" << YAML::Value << m.DoubleSided;

	out << YAML::EndMap;
	out << YAML::EndMap;

	std::ofstream fout(path);
	fout << out.c_str();
}


ShaderDesc ShaderSerializer::Deserialize(const std::filesystem::path& path)
{
	YAML::Node root = YAML::LoadFile(path.string());
	auto shader = root["Shader"];

	ShaderDesc desc;
	desc.VertexPath   = shader["Vertex"].as<std::string>();
	desc.FragmentPath = shader["Fragment"].as<std::string>();
	return desc;
}
