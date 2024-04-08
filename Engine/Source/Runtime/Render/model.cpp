#include "model.h"

#include <iostream>

#include "Resource/texture.h"

namespace gdp1 {

Model::Model(const std::string const& path, const std::string& shader, const std::vector<TexturesDesc> textures,
             unsigned int instancing, std::vector<glm::mat4> instanceMatrix,
             bool gamma /*= false*/)
    : gammaCorrection(gamma)
    , shaderName(shader)
    , num_vertices_(0)
    , num_triangles_(0)
    , currentAnimation(nullptr)
    , texturesToLoad(textures)
    , instanceMatrix(instanceMatrix)
    , instancing(instancing) {
    LoadModel(path);
}

// Copy constructor
Model::Model(const Model& other)
    : textures_loaded(other.textures_loaded)
    , meshes(other.meshes)
    , directory(other.directory)
    , gammaCorrection(other.gammaCorrection)
    , shaderName(other.shaderName)
    , bounds(other.bounds)
    , m_global_inverse_transform(other.m_global_inverse_transform)
    , character_animations(other.character_animations)
    , currentAnimation(other.currentAnimation)
    , prevAnimation(other.prevAnimation)
    , num_vertices_(other.num_vertices_)
    , num_triangles_(other.num_triangles_)
    , m_bone_mapping(other.m_bone_mapping)
    , m_num_bones(other.m_num_bones)
    , m_bone_matrices(other.m_bone_matrices)
    , elapsedTime(other.elapsedTime)
    , texturesToLoad(other.texturesToLoad)
    , scene(other.scene) {}

void Model::Draw(Shader* shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].Draw(shader);
}

void Model::DrawDebug(Shader* shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].DrawDebug(shader);
}

void Model::SetupInstancing(std::vector<glm::mat4>& instanceMatrix) {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].SetupInstancing(instanceMatrix);
}

void Model::ResetInstancing() {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].ResetInstancing();
}

unsigned int Model::GetVertexCount() const { return num_vertices_; }

unsigned int Model::GetTriangleCount() const { return num_triangles_; }

void Model::AddCharacterAnimation(std::string animationName, std::string animationPath) {
    CharacterAnimation* animation = new CharacterAnimation(animationPath, animationName, this);
    character_animations[animationName] = animation;
}

uint32_t Model::GetAnimationIndex(CharacterAnimation* animation) {
    uint32_t index = 0;
    for (const auto& pair : character_animations) {
        if (pair.second == animation) {
            return index;
        }
        ++index;
    }

    return -1;
}

void Model::UpdateAnimation(float deltaTime) {
    blendFactor += 0.001f;
    elapsedTime += deltaTime;

    if (blendFactor > 1.0f) {
        blendFactor = 1.0f;
        prevAnimation = currentAnimation;
    }
}

void Model::SetCurrentAnimation(std::string name) {
    auto it = character_animations.find(name);
    if (it != character_animations.end()) {
        if (currentAnimation == nullptr) prevAnimation = it->second;
        currentAnimation = it->second;
        blendFactor = 0.0f;
        elapsedTime = 0.0f;
    } else {
        currentAnimation = nullptr;
    }
}

void Model::LoadModel(std::string const& path) {
    // read file via ASSIMP
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                        aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);

    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)  // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    m_global_inverse_transform = scene->mRootNode->mTransformation;
    m_global_inverse_transform.Inverse();

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
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.SetBoneDefaults();

        glm::vec3
            vector;  // we declare a placeholder vector since assimp uses its own vector class that doesn't directly
        // convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        glm::vec4 color;
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0])  // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        } else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding
    // vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
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
    // 2. specular maps
    std::vector<TextureInfo> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    //  3. normal maps
    std::vector<TextureInfo> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<TextureInfo> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    // 5. opacity maps
    std::vector<TextureInfo> opacityMaps = LoadMaterialTextures(material, aiTextureType_OPACITY, "texture_opacity");
    textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());

    for (TexturesDesc textureDesc : texturesToLoad) {
        TextureInfo* texture = LoadTexture(textureDesc.name, textureDesc.type);
        if (texture != nullptr) textures.push_back(*texture);
    }

    // load bones
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        unsigned int bone_index = 0;
        std::string bone_name(mesh->mBones[i]->mName.data);

        if (m_bone_mapping.find(bone_name) == m_bone_mapping.end()) {
            // Allocate an index for a new bone
            bone_index = m_num_bones;
            m_num_bones++;

            BoneMatrix bi;
            bi.offset_matrix = mesh->mBones[i]->mOffsetMatrix;
            m_bone_matrices.push_back(bi);
            m_bone_mapping[bone_name] = bone_index;

        } else {
            bone_index = m_bone_mapping[bone_name];
        }

        for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
            unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
            float weight = mesh->mBones[i]->mWeights[j].mWeight;
            vertices[vertex_id].AddBoneData(bone_index, weight);
        }
    }

    // return a mesh object created from the extracted mesh data
    aiAABB& aabb = mesh->mAABB;
    Bounds b;
    b.SetMinMax(glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z), glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z));
    Mesh m = Mesh(vertices, indices, textures, b);

    bounds.Expand(m.bounds);  // expand model's bounding box so it contains all meshes

    return m;
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

TextureInfo* Model::LoadTexture(std::string textureName, std::string textureType) {
    TextureInfo* texture = nullptr;

    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
        if (textures_loaded[j].path == textureName) {
            skip = true;
            break;
        }
    }

    if (!skip) {
        texture = new TextureInfo();
        texture->id = Texture::LoadTexture(textureName);
        texture->type = textureType;
        texture->path = textureName;

        textures_loaded.push_back(*texture);
    }

    return texture;
}

std::map<std::string, unsigned int>& Model::GetBoneMap() { return m_bone_mapping; }

std::vector<BoneMatrix>& Model::GetBoneMatrices() { return m_bone_matrices; }

unsigned int Model::GetNumBones() { return m_num_bones; }

}  // namespace gdp1
