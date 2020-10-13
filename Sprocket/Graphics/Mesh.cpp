#include "Mesh.h"

#include <cassert>
#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Sprocket {
namespace {

Maths::mat4 Convert(const aiMatrix4x4& matrix)
{
    Maths::mat4 result;

    result[0][0] = matrix.a1;
    result[0][1] = matrix.b1;
    result[0][2] = matrix.c1;
    result[0][3] = matrix.d1;

    result[1][0] = matrix.a2;
    result[1][1] = matrix.b2;
    result[1][2] = matrix.c2;
    result[1][3] = matrix.d2;

    result[2][0] = matrix.a3;
    result[2][1] = matrix.b3;
    result[2][2] = matrix.c3;
    result[2][3] = matrix.d3;

    result[3][0] = matrix.a4;
    result[3][1] = matrix.b4;
    result[3][2] = matrix.c4;
    result[3][3] = matrix.d4;

    return result;
}


Maths::vec2 Convert(const aiVector2D& v)
{
    return Maths::vec2{v.x, v.y};
}

Maths::vec3 Convert(const aiVector3D& v)
{
    return Maths::vec3{v.x, v.y, v.z};
}

bool IsSceneValid(const aiScene* scene)
    // Returns true if the scene is valid and false otherwise.
{
    return scene && 
           !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) &&
           scene->mRootNode &&
           scene->mNumMeshes > 0;
}

int GetAssimpFlags()
{
    return aiProcess_Triangulate
         | aiProcess_FlipUVs
         | aiProcess_CalcTangentSpace
         | aiProcess_GenUVCoords
         | aiProcess_GenNormals
         | aiProcess_ValidateDataStructure;
}

void AddBoneData(AnimVertex& vertex, std::uint32_t index, float weight)
{
    for (std::uint32_t i = 0; i != 4; ++i) {
        if (vertex.boneIndices[i] == -1) {
            vertex.boneIndices[i] = index;
            vertex.boneWeights[i] = weight;
            return;
        }
    }
}

std::shared_ptr<Mesh> LoadStaticMesh(const aiScene* scene)
{    
    StaticMeshData data;

    for (std::size_t idx = 0; idx != scene->mNumMeshes; ++idx) {
        aiMesh* mesh = scene->mMeshes[idx];

        // Vertices
        for (unsigned int i = 0; i != mesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.position = Convert(mesh->mVertices[i]);
            vertex.normal = Convert(mesh->mNormals[i]);
            vertex.textureCoords = Convert(mesh->mTextureCoords[0][i]);
            
            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent = Convert(mesh->mTangents[i]);
                vertex.bitangent = Convert(mesh->mBitangents[i]);
            }

            data.vertices.push_back(vertex);
        }

        // Indices
        for (unsigned int i = 0; i != mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j != face.mNumIndices; ++j) {
                data.indices.push_back(face.mIndices[j]);
            }
        }
    }
    return std::make_shared<Mesh>(data);
}

std::shared_ptr<Mesh> LoadAnimatedMesh(const aiScene* scene)
{    
    AnimatedMeshData data;
    auto& skel = data.skeleton;
    SPKT_LOG_INFO("Loading animated mesh");

    std::uint32_t vertexCount = 0;

    for (std::size_t idx = 0; idx != scene->mNumMeshes; ++idx) {
        aiMesh* mesh = scene->mMeshes[idx];

        // Vertices
        for (std::uint32_t i = 0; i != mesh->mNumVertices; ++i) {
            AnimVertex vertex;
            vertex.position = Convert(mesh->mVertices[i]);
            vertex.normal = Convert(mesh->mNormals[i]);
            vertex.textureCoords = Convert(mesh->mTextureCoords[0][i]);
            
            if (mesh->HasTangentsAndBitangents()) {
                vertex.tangent = Convert(mesh->mTangents[i]);
                vertex.bitangent = Convert(mesh->mBitangents[i]);
            }

            data.vertices.push_back(vertex);
        }

        // Indices
        for (std::uint32_t i = 0; i != mesh->mNumFaces; ++i) {
            aiFace face = mesh->mFaces[i];
            for (std::uint32_t j = 0; j != face.mNumIndices; ++j) {
                data.indices.push_back(face.mIndices[j]);
            }
        }

        // Bones
        for (std::uint32_t i = 0; i != mesh->mNumBones; ++i) {
            aiBone* bone = mesh->mBones[i];
            std::string boneName(bone->mName.data);

            // We have to do this lookup as a bone be shared by multiple
            // submeshes, so we may have already encountered this bone.
            int boneIndex = 0;
            if (auto it = skel.boneMap.find(boneName); it != skel.boneMap.end()) {
                boneIndex = it->second;
            } else {
                boneIndex = data.skeleton.bones.size();
                Bone newBone;
                newBone.transform = Convert(bone->mOffsetMatrix);
                skel.bones.push_back(newBone);
                skel.boneMap[boneName] = boneIndex;
            }

            // Update Vertices
            for (std::uint32_t j = 0; j != bone->mNumWeights; ++j) {
                auto& vertex = data.vertices[vertexCount + bone->mWeights[j].mVertexId];
                float weight = bone->mWeights[j].mWeight;
                AddBoneData(vertex, boneIndex, weight);
            }
        }

        vertexCount += mesh->mNumVertices;
    }
    return std::make_shared<Mesh>(data);
}

}

Mesh::Mesh(const StaticMeshData& data)
    : d_vertexBuffer(0)
    , d_indexBuffer(0)
    , d_vertexCount(data.indices.size())
    , d_layout(sizeof(Vertex), 0)
    , d_animated(false)
{
    glCreateBuffers(1, &d_vertexBuffer);
    glNamedBufferData(d_vertexBuffer, sizeof(Vertex) * data.vertices.size(), data.vertices.data(), GL_STATIC_DRAW);

    glCreateBuffers(1, &d_indexBuffer);
    glNamedBufferData(d_indexBuffer, sizeof(std::uint32_t) * data.indices.size(), data.indices.data(), GL_STATIC_DRAW);

    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 2);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    assert(d_layout.Validate());
}

Mesh::Mesh(const AnimatedMeshData& data)
    : d_vertexBuffer(0)
    , d_indexBuffer(0)
    , d_vertexCount(data.indices.size())
    , d_layout(sizeof(AnimVertex), 0)
    , d_animated(true)
    , d_skeleton(data.skeleton)
{
    glCreateBuffers(1, &d_vertexBuffer);
    glNamedBufferData(d_vertexBuffer, sizeof(AnimVertex) * data.vertices.size(), data.vertices.data(), GL_STATIC_DRAW);

    glCreateBuffers(1, &d_indexBuffer);
    glNamedBufferData(d_indexBuffer, sizeof(std::uint32_t) * data.indices.size(), data.indices.data(), GL_STATIC_DRAW);

    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 2);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::INT,   4);
    d_layout.AddAttribute(DataType::FLOAT, 4);
    assert(d_layout.Validate());
}

Mesh::Mesh()
    : d_vertexBuffer(0)
    , d_indexBuffer(0)
    , d_vertexCount(0)
    , d_layout(sizeof(Vertex), 0)
    , d_animated(false)
{
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 2);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    d_layout.AddAttribute(DataType::FLOAT, 3);
    assert(d_layout.Validate());
}

Mesh::~Mesh()
{
    if (d_vertexBuffer != 0) {
        glDeleteBuffers(1, &d_vertexBuffer);
    }
    if (d_indexBuffer != 0) {
        glDeleteBuffers(1, &d_indexBuffer);
    }
}

std::shared_ptr<Mesh> Mesh::FromFile(const std::string& file)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, GetAssimpFlags());

    if (!IsSceneValid(scene)) {
        SPKT_LOG_ERROR("ERROR::ASSIMP::{}", importer.GetErrorString());
        return std::make_shared<Mesh>();
    }

    if (scene->HasAnimations()) {
        return LoadAnimatedMesh(scene);
    }
    return LoadStaticMesh(scene);
}

void Mesh::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, d_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_indexBuffer);
    d_layout.SetAttributes();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool Mesh::operator==(const Mesh& other) const
{
    // Two models are the same if they point to the same VAO.
    return d_vertexBuffer == other.d_vertexBuffer &&
           d_indexBuffer == other.d_indexBuffer;
}

BufferLayout Mesh::GetLayout() const
{
    return d_layout;
}

void Mesh::SetPose(const std::string& name, float time)
{

}

std::vector<std::string> Mesh::GetAnimationNames() const
{
    return {};
}

}