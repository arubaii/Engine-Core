#include <yaml-cpp/yaml.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <stdexcept>
#include "ModelImporter.h"
#include "asset_core/AssetManager.h"


static MaterialDesc ExtractPBRMaterial(const aiMaterial* src, const std::filesystem::path& modelDir)
{
    MaterialDesc desc;

    // Base Color Factor
    aiColor4D baseColor;
    if (AI_SUCCESS == src->Get(AI_MATKEY_BASE_COLOR, baseColor))
    {
        desc.BaseColorFactor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    }
    else if (AI_SUCCESS == src->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor))
    {
        desc.BaseColorFactor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, 1.0f);
    }

    // Metallic & Roughness Factors
    float metallic  = 1.0f;
    float roughness = 1.0f;

    src->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    src->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

    // Fallback for older formats
    float shininess = 0.0f;
    if (src->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
    {
        roughness = 1.0f - glm::sqrt(shininess / 1000.0f);
    }

    desc.MetallicFactor  = metallic;
    desc.RoughnessFactor = roughness;

    // Emissive Factor
    aiColor3D emissive(0, 0, 0);
    if (AI_SUCCESS == src->Get(AI_MATKEY_COLOR_EMISSIVE, emissive))
    {
        desc.EmissiveFactor = glm::vec3(emissive.r, emissive.g, emissive.b);
    }

    // Alpha Mode
    int blend = 0;
    if (src->Get(AI_MATKEY_BLEND_FUNC, blend) == AI_SUCCESS)
    {
        desc.AlphaMode = (blend == aiBlendMode_Default ? "OPAQUE" : "BLEND");
    }

    //  Double Sided
    bool twoSided = false;
    src->Get(AI_MATKEY_TWOSIDED, twoSided);
    desc.DoubleSided = twoSided;

    //  TEXTURE IMPORT HELPERS
    auto ImportTexture = [&](aiTextureType type) -> AssetHandle
    {
        if (src->GetTextureCount(type) == 0)
            return AssetHandle(0);

        aiString texPath;
        if (src->GetTexture(type, 0, &texPath) != AI_SUCCESS)
            return AssetHandle(0);

        std::filesystem::path fullPath = modelDir / texPath.C_Str();
        if (!std::filesystem::exists(fullPath))
            return AssetHandle(0);

        return AssetManager::ImportAsset(fullPath);
    };

    // ============
    // PBR textures
    // ============

    desc.BaseColorTexture = ImportTexture(aiTextureType_BASE_COLOR);
    if (!desc.BaseColorTexture)
        desc.BaseColorTexture = ImportTexture(aiTextureType_DIFFUSE);

    desc.NormalTexture = ImportTexture(aiTextureType_NORMALS);

    // Metallic/Roughness: try the usual Assimp slots first
    desc.MetallicRoughnessTexture = ImportTexture(aiTextureType_METALNESS);
    if (!desc.MetallicRoughnessTexture)
        desc.MetallicRoughnessTexture = ImportTexture(aiTextureType_DIFFUSE_ROUGHNESS);

    // Last resort
    if (!desc.MetallicRoughnessTexture)
        desc.MetallicRoughnessTexture = ImportTexture(aiTextureType_UNKNOWN);

    // Occlusion: prefer AO if available, fall back to LIGHTMAP
    desc.OcclusionTexture = ImportTexture(aiTextureType_AMBIENT_OCCLUSION);
    if (!desc.OcclusionTexture)
        desc.OcclusionTexture = ImportTexture(aiTextureType_LIGHTMAP);

    desc.EmissiveTexture = ImportTexture(aiTextureType_EMISSIVE);

    // Legacy fallback (OBJ/FBX/etc)
    if (!desc.BaseColorTexture)
        desc.BaseColorTexture = ImportTexture(aiTextureType_DIFFUSE);

    return desc;
}


static void WriteMeshDescriptor
(
    const std::filesystem::path& outMeshPath,
    const std::filesystem::path& sourceModelPath,
    uint32_t meshIndex
)
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Mesh" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Source" << YAML::Value << sourceModelPath.string();
    out << YAML::Key << "MeshIndex" << YAML::Value << meshIndex;
    out << YAML::EndMap;
    out << YAML::EndMap;

    std::ofstream file(outMeshPath);
    if (!file.is_open())
        throw std::runtime_error("ModelImporter: failed to write " + outMeshPath.string());
    file << out.c_str();
}

static void WriteMaterialStub(const std::filesystem::path& outMatPath)
{
    // Must match MaterialSerializer::Deserialize:
    // Material:
    //   Shader: <AssetHandle>
    //   Albedo: <AssetHandle>
    //   Color: [r,g,b]
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Material" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "Shader" << YAML::Value << (AssetHandle)0;
    out << YAML::Key << "Albedo" << YAML::Value << (AssetHandle)0;
    out << YAML::Key << "Color"  << YAML::Value << YAML::Flow << YAML::BeginSeq << 1.0f << 1.0f << 1.0f << YAML::EndSeq;
    out << YAML::EndMap;
    out << YAML::EndMap;

    std::ofstream file(outMatPath);
    if (!file.is_open())
        throw std::runtime_error("ModelImporter: failed to write " + outMatPath.string());
    file << out.c_str();
}

static void WriteModelFile(
    const std::filesystem::path& outPath,
    const ModelImportData& data)
{
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "Model" << YAML::Value;
    out << YAML::BeginMap;

    out << YAML::Key << "Submeshes" << YAML::Value << YAML::BeginSeq;

    for (auto& sm : data.Submeshes)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "Mesh"        << YAML::Value << (uint64_t)sm.Mesh;
        out << YAML::Key << "Material"    << YAML::Value << (uint64_t)sm.Material;
        out << YAML::Key << "IndexOffset" << YAML::Value << sm.IndexOffset;
        out << YAML::Key << "IndexCount"  << YAML::Value << sm.IndexCount;
        out << YAML::EndMap;
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;
    out << YAML::EndMap;

    std::ofstream file(outPath);
    file << out.c_str();
}


ModelImportData ModelImporter::Import(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        throw std::runtime_error("ModelImporter: file does not exist: " + path.string());

    Assimp::Importer importer;

    const std::string ext = path.extension().string();
    const bool isGLTF = (ext == ".gltf" || ext == ".glb");

    unsigned int flags =
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_ImproveCacheLocality |
        aiProcess_SortByPType |
        aiProcess_ValidateDataStructure;

    if (!isGLTF)
    {
        flags |=
            aiProcess_JoinIdenticalVertices |
            aiProcess_OptimizeMeshes |
            aiProcess_OptimizeGraph;
    }

    const aiScene* scene = importer.ReadFile(path.string(), flags);

    if (!scene || !scene->HasMeshes())
        throw std::runtime_error("ModelImporter: Assimp failed for " + path.string());

    ModelImportData result;

    // Directory for generated asset descriptor files
    const std::filesystem::path outDir =
        path.parent_path() / (path.stem().string() + "_import");

    std::filesystem::create_directories(outDir);

    // Path used to resolve texture files
    const std::filesystem::path modelDir = path.parent_path();

    for (uint32_t i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* aiMeshRef = scene->mMeshes[i];
        aiMaterial* aiMat = scene->mMaterials[aiMeshRef->mMaterialIndex];

        const std::filesystem::path meshDescPath = outDir / ("mesh_" + std::to_string(i) + ".mesh");
        WriteMeshDescriptor(meshDescPath, path, i);

        AssetHandle meshHandle = AssetManager::ImportAsset(meshDescPath);
        MaterialDesc matDesc = ExtractPBRMaterial(aiMat, modelDir);

        // Write .mat file
        const std::filesystem::path matDescPath = outDir / ("mat_" + std::to_string(i) + ".mat");
        MaterialSerializer::Serialize(matDescPath, matDesc);

        // Import material asset
        AssetHandle matHandle = AssetManager::ImportAsset(matDescPath);

        // Compute IndexCount
        uint32_t indexCount = 0;
        for (unsigned f = 0; f < aiMeshRef->mNumFaces; f++)
        {
            if (aiMeshRef->mFaces[f].mNumIndices == 3)
                indexCount += 3;
        }

        // Fill Submesh
        ModelImportData::Submesh sm;
        sm.Mesh        = meshHandle;
        sm.Material    = matHandle;
        sm.IndexOffset = 0;
        sm.IndexCount  = indexCount;

        result.Submeshes.push_back(sm);
    }

    std::filesystem::path modelPath =
    path.parent_path() / (path.stem().string() + ".model");

    WriteModelFile(modelPath, result);

    // Register with AssetManager
    AssetHandle modelHandle = AssetManager::ImportAsset(modelPath);
    result.Model = modelHandle;

    return result;
}
