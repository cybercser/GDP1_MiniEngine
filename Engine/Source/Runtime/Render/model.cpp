#include "model.h"

#include <iostream>
#include <sstream>

#include "Resource/texture.h"
#include "Animation/animator.h"

namespace gdp1 {

#define ASSIMP_PREPROCESS_FLAGS aiProcess_Triangulate | aiProcess_FlipUVs

Model::Model(const std::string const& path, const std::string& shader, bool gamma /*= false*/)
    : gammaCorrection(gamma)
    , shaderName(shader)
    , num_vertices_(0)
    , num_triangles_(0)
    , animator(0) {
    LoadModel(path);
}

void Model::Draw(Shader* shader) {
    if (animator) {
        auto transforms = animator->GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i) {
            shader->SetUniform("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
    }

    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].Draw(shader);
}

void Model::DrawDebug(Shader* shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].DrawDebug(shader);
}

unsigned int Model::GetVertexCount() const { return num_vertices_; }

unsigned int Model::GetTriangleCount() const { return num_triangles_; }

unsigned int Model::GetAnimationsCount() const { return character_animations.size(); }

void Model::UpdateCharacterAnimation(float deltaTime) {
    if (animator) {
        animator->UpdateAnimation(deltaTime);
    }
}

bool Model::SetCurrentAnimation(const std::string& animationName) {
    auto iterAni = character_animations.find(animationName);
    if (iterAni == character_animations.end()) {
        return false;
    }

    if (animator) {
        animator->PlayAnimation(iterAni->second);
    } else {
        animator = new Animator(iterAni->second);
    }

    return true;
}

bool Model::AddAnimation(const std::string& animationName, CharacterAnimation* animation) {
    if (character_animations.find(animationName) != character_animations.end()) {
        LOG_ERROR("Trying to add an animation that already exists: ", animationName);
        return false;
    }

    character_animations[animationName] = animation;
}

void Model::LoadModel(std::string const& path) {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, ASSIMP_PREPROCESS_FLAGS);

    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)  // if is Not Zero
    {
        LOG_ERROR("ERROR::ASSIMP:: ", importer.GetErrorString());
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene);

    LOG_INFO("Loaded model {0} with {1} meshes", path, meshes.size());
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene_ptr_.
        // the scene_ptr_ contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));

        num_vertices_ += mesh->mNumVertices;
        num_triangles_ += mesh->mNumFaces;
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<TextureInfo> textures;

    baseVertex = vertices.size();
    baseIndex = indices.size();

    size_t numVertices = mesh->mNumVertices;
    size_t numIndices = mesh->mNumFaces * 3;

    vertices.resize(baseVertex + numVertices);
    indices.resize(baseIndex + numIndices);

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex& vertex = vertices[baseVertex + i];
        vertex.SetBoneDefaults();

        // positions
        convert::ToGLM(mesh->mVertices[i], vertex.position);

        if (mesh->mNormals) {
            convert::ToGLM(mesh->mNormals[i], vertex.normal);
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents()) {
            convert::ToGLM(mesh->mTangents[i], vertex.tangent);
            convert::ToGLM(mesh->mBitangents[i], vertex.bitangent);
        }

        if (mesh->HasTextureCoords(0)) {
            convert::ToGLM(mesh->mTextureCoords[0][i], vertex.texCoords); 
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }
    }

    // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding
    // vertex indices.
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (size_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<TextureInfo> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    //// 2. specular maps
    // std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    // textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    //  3. normal maps
    std::vector<TextureInfo> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<TextureInfo> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    // 5. opacity maps
    std::vector<TextureInfo> opacityMaps = LoadMaterialTextures(material, aiTextureType_OPACITY, "texture_opacity");
    textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());

    if (mesh->HasBones()) {
        ProcessMeshBones(baseVertex, mesh, scene);
    }

    // return a mesh object created from the extracted mesh data
    aiAABB& aabb = mesh->mAABB;
    Bounds b;
    b.SetMinMax(glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z), glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z));

    Mesh m = Mesh(baseVertex, baseIndex, vertices, indices, textures, b);
    bounds.Expand(m.bounds);  // expand model's bounding box so it contains all meshes

    return m;
}

void Model::ProcessMeshBones(size_t baseVertex, aiMesh* mesh, const aiScene* scene) {
    auto& boneInfoMap = this->boneInfoMap;
    int& boneCount = this->bone_counter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            sBoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            convert::ToGLM(mesh->mBones[boneIndex]->mOffsetMatrix, newBoneInfo.offset);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        } else {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(baseVertex + vertexId < vertices.size());
            vertices[baseVertex + vertexId].AddBoneData(boneID, weight);
        }
    }
}

std::vector<TextureInfo> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
    std::vector<TextureInfo> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;  // a texture with the same filepath has already been loaded, continue to next one.
                // (optimization)
                break;
            }
        }
        if (!skip) {  // if texture hasn't been loaded already, load it
            TextureInfo texture;
            // texture.id = TextureFromFile(str.C_Str(), this->directory);
            std::string texturePath = this->directory + '/' + str.C_Str();
            texture.id = Texture::LoadTexture(texturePath);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't
                                                 // unnecessary load duplicate textures.
        }
    }
    return textures;
}

std::map<std::string, sBoneInfo>& Model::GetBoneInfoMap() { return boneInfoMap; }

int& Model::GetBoneCount() { return bone_counter; }

}  // namespace gdp1
