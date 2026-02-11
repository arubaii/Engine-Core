#include <yaml-cpp/yaml.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <glm/vec3.hpp>
#include "MeshImporter.h"
#include "renderer_core/Mesh.h"
#include "renderer_core/MeshUtils.h"
#include "utils/Log.h"

static void PushVec3(std::vector<float>& out, float x, float y, float z)
{
    out.push_back(x); out.push_back(y); out.push_back(z);
}

static void PushVec2(std::vector<float>& out, float x, float y)
{
    out.push_back(x); out.push_back(y);
}

MeshData MeshImporter::Import(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
        throw std::runtime_error("MeshImporter: file does not exist: " + path.string());

    if (path.extension() != ".mesh")
        throw std::runtime_error("MeshImporter: expected .mesh descriptor, got: " + path.string());

    YAML::Node root = YAML::LoadFile(path.string());
    YAML::Node meshNode = root["Mesh"];

    const std::string source    = meshNode["Source"].as<std::string>();
    const uint32_t    meshIndex = meshNode["MeshIndex"].as<uint32_t>();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        source,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs
    );

    if (!scene || !scene->HasMeshes())
        throw std::runtime_error("MeshImporter: scene invalid: " + source);

    aiMesh* m = scene->mMeshes[meshIndex];


    bool hadUVs = m->HasTextureCoords(0);
    bool hadTangents = m->HasTangentsAndBitangents();

    MeshData data;

    data.Vertices.reserve(m->mNumVertices * 3);
    data.Normals.reserve(m->mNumVertices * 3);
    data.TexCoords.reserve(m->mNumVertices * 2);
    data.Colors.reserve(m->mNumVertices * 3);
    data.Tangents.reserve(m->mNumVertices * 3);
    data.Bitangents.reserve(m->mNumVertices * 3);
    data.Indices.reserve(m->mNumFaces * 3);

    // Compute centroid for recentering
    glm::vec3 centroid(0.0f);
    for (unsigned i = 0; i < m->mNumVertices; i++)
        centroid += glm::vec3(m->mVertices[i].x, m->mVertices[i].y, m->mVertices[i].z);
    centroid /= float(m->mNumVertices);

    // Fill vertex attributes
    for (unsigned i = 0; i < m->mNumVertices; i++)
    {
        glm::vec3 pos(
            m->mVertices[i].x - centroid.x,
            m->mVertices[i].y - centroid.y,
            m->mVertices[i].z - centroid.z
        );

        data.Vertices.push_back(pos.x);
        data.Vertices.push_back(pos.y);
        data.Vertices.push_back(pos.z);

        // Normal
        if (m->HasNormals()) {
            data.Normals.push_back(m->mNormals[i].x);
            data.Normals.push_back(m->mNormals[i].y);
            data.Normals.push_back(m->mNormals[i].z);
        } else {
            data.Normals.insert(data.Normals.end(), {0,0,1});
        }

        // UVs
        if (hadUVs) {
            data.TexCoords.push_back(m->mTextureCoords[0][i].x);
            data.TexCoords.push_back(m->mTextureCoords[0][i].y);
        } else {
            // Sentinel (we overwrite this later)
            data.TexCoords.push_back(-9999.0f);
            data.TexCoords.push_back(-9999.0f);
        }

        // Color (default)
        data.Colors.push_back(1.0f);
        data.Colors.push_back(1.0f);
        data.Colors.push_back(1.0f);

        // Tangent space
        if (hadTangents) {
            data.Tangents.push_back(m->mTangents[i].x);
            data.Tangents.push_back(m->mTangents[i].y);
            data.Tangents.push_back(m->mTangents[i].z);

            data.Bitangents.push_back(m->mBitangents[i].x);
            data.Bitangents.push_back(m->mBitangents[i].y);
            data.Bitangents.push_back(m->mBitangents[i].z);

        }
        else {
            data.Tangents.insert(data.Tangents.end(), {0,0,0});
            data.Bitangents.insert(data.Bitangents.end(), {0,0,0});
        }
    }

    // Indices
    for (unsigned f = 0; f < m->mNumFaces; f++)
    {
        const aiFace& face = m->mFaces[f];
        if (face.mNumIndices == 3)
        {
            data.Indices.push_back(face.mIndices[0]);
            data.Indices.push_back(face.mIndices[1]);
            data.Indices.push_back(face.mIndices[2]);
        }
    }

    if (!hadUVs)
        MeshUtils::GenerateSphericalUVs(data);

    // If Assimp did not provide them
    if (!hadTangents || !hadUVs)
        MeshUtils::GenerateTangents(data);


    return data;
}

