#include "model.h"

#include <iostream>

#include "Resource/texture.h"

namespace gdp1 {

glm::mat4 aiMatrix4x4ToGlmMat4(const aiMatrix4x4& matrix) {
    glm::mat4 result;

    // Copy elements from aiMatrix4x4 to glm::mat4
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

Model::Model(const std::string const& path, const std::string& shader, bool gamma /*= false*/)
    : gammaCorrection(gamma)
    , shaderName(shader)
    , num_vertices_(0)
    , num_triangles_(0) {
    LoadModel(path);
    m_bone_location[0] = -1;
}

void Model::Draw(Shader* shader) {
    if (m_bone_matrices.size() > 0) {
        std::vector<aiMatrix4x4> transforms;
        elapsedTime += 0.01f;
        boneTransform(elapsedTime, transforms);

        shader->SetUniform("u_HasBones", true);

        for (unsigned int i = 0; i < transforms.size(); i++) {
            std::string name = "bones[" + std::to_string(i) + "]";
            shader->SetUniform(name, aiMatrix4x4ToGlmMat4(transforms[i]));
        }
    }

    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].Draw(shader);
}

void Model::DrawDebug(Shader* shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].DrawDebug(shader);
}

unsigned int Model::GetVertexCount() const { return num_vertices_; }

unsigned int Model::GetTriangleCount() const { return num_triangles_; }

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

    if (scene->HasAnimations()) {
        if (scene->mAnimations[0]->mTicksPerSecond != 0.0) {
            ticks_per_second = scene->mAnimations[0]->mTicksPerSecond;
            duration = scene->mAnimations[0]->mDuration;
        } else {
            ticks_per_second = 25.0f;
        }
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
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
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

    // load bones
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        unsigned int bone_index = 0;
        std::string bone_name(mesh->mBones[i]->mName.data);

        std::cout << mesh->mBones[i]->mName.data << std::endl;

        if (m_bone_mapping.find(bone_name) == m_bone_mapping.end()) {
            // Allocate an index for a new bone
            bone_index = m_num_bones;
            m_num_bones++;

            BoneMatrix bi;
            m_bone_matrices.push_back(bi);
            m_bone_matrices[bone_index].offset_matrix = mesh->mBones[i]->mOffsetMatrix;
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

unsigned int Model::findPosition(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumPositionKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mPositionKeys[i + 1].mTime) return i;
    }

    assert(0);
    return 0;
}

unsigned int Model::findRotation(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumRotationKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mRotationKeys[i + 1].mTime) return i;
    }

    assert(0);
    return 0;
}

unsigned int Model::findScaling(float p_animation_time, const aiNodeAnim* p_node_anim) {
    for (unsigned int i = 0; i < p_node_anim->mNumScalingKeys - 1; i++) {
        if (p_animation_time < (float)p_node_anim->mScalingKeys[i + 1].mTime) return i;
    }

    assert(0);
    return 0;
}

aiVector3D Model::calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim) {
    if (p_node_anim->mNumPositionKeys == 1) {
        return p_node_anim->mPositionKeys[0].mValue;
    }

    unsigned int position_index = findPosition(p_animation_time, p_node_anim);
    unsigned int next_position_index = position_index + 1;
    assert(next_position_index < p_node_anim->mNumPositionKeys);
    float delta_time = (float)(p_node_anim->mPositionKeys[next_position_index].mTime -
                               p_node_anim->mPositionKeys[position_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mPositionKeys[position_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    aiVector3D start = p_node_anim->mPositionKeys[position_index].mValue;
    aiVector3D end = p_node_anim->mPositionKeys[next_position_index].mValue;
    aiVector3D delta = end - start;

    return start + factor * delta;
}

aiQuaternion Model::calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim) {
    if (p_node_anim->mNumRotationKeys == 1) {
        return p_node_anim->mRotationKeys[0].mValue;
    }

    unsigned int rotation_index = findRotation(p_animation_time, p_node_anim);
    unsigned int next_rotation_index = rotation_index + 1;
    assert(next_rotation_index < p_node_anim->mNumRotationKeys);
    float delta_time = (float)(p_node_anim->mRotationKeys[next_rotation_index].mTime -
                               p_node_anim->mRotationKeys[rotation_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    aiQuaternion start_quat = p_node_anim->mRotationKeys[rotation_index].mValue;
    aiQuaternion end_quat = p_node_anim->mRotationKeys[next_rotation_index].mValue;

    return nlerp(start_quat, end_quat, factor);
}

aiVector3D Model::calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim) {
    if (p_node_anim->mNumScalingKeys == 1) {
        return p_node_anim->mScalingKeys[0].mValue;
    }

    unsigned int scaling_index = findScaling(p_animation_time, p_node_anim);
    unsigned int next_scaling_index = scaling_index + 1;
    assert(next_scaling_index < p_node_anim->mNumScalingKeys);
    float delta_time =
        (float)(p_node_anim->mScalingKeys[next_scaling_index].mTime - p_node_anim->mScalingKeys[scaling_index].mTime);
    float factor = (p_animation_time - (float)p_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    aiVector3D start = p_node_anim->mScalingKeys[scaling_index].mValue;
    aiVector3D end = p_node_anim->mScalingKeys[next_scaling_index].mValue;
    aiVector3D delta = end - start;

    return start + factor * delta;
}

const aiNodeAnim* Model::findNodeAnim(const aiAnimation* p_animation, const std::string p_node_name) {
    for (unsigned int i = 0; i < p_animation->mNumChannels; i++) {
        const aiNodeAnim* node_anim = p_animation->mChannels[i];
        if (std::string(node_anim->mNodeName.data) == p_node_name) {
            return node_anim;
        }
    }

    return nullptr;
}
// start from RootNode
void Model::readNodeHierarchy(float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform) {
    std::string node_name(p_node->mName.data);

    const aiAnimation* animation = scene->mAnimations[0];
    aiMatrix4x4 node_transform = p_node->mTransformation;

    const aiNodeAnim* node_anim = findNodeAnim(animation, node_name);

    if (node_anim) {
        // scaling
        // aiVector3D scaling_vector = node_anim->mScalingKeys[2].mValue;
        aiVector3D scaling_vector = calcInterpolatedScaling(p_animation_time, node_anim);
        aiMatrix4x4 scaling_matr;
        aiMatrix4x4::Scaling(scaling_vector, scaling_matr);

        // rotation
        // aiQuaternion rotate_quat = node_anim->mRotationKeys[2].mValue;
        aiQuaternion rotate_quat = calcInterpolatedRotation(p_animation_time, node_anim);
        aiMatrix4x4 rotate_matr = aiMatrix4x4(rotate_quat.GetMatrix());

        // translation
        // aiVector3D translate_vector = node_anim->mPositionKeys[2].mValue;
        aiVector3D translate_vector = calcInterpolatedPosition(p_animation_time, node_anim);
        aiMatrix4x4 translate_matr;
        aiMatrix4x4::Translation(translate_vector, translate_matr);

        node_transform = translate_matr * rotate_matr * scaling_matr;
    }

    aiMatrix4x4 global_transform = parent_transform * node_transform;

    if (m_bone_mapping.find(node_name) != m_bone_mapping.end())  // true if node_name exist in bone_mapping
    {
        unsigned int bone_index = m_bone_mapping[node_name];
        m_bone_matrices[bone_index].final_world_transform =
            m_global_inverse_transform * global_transform * m_bone_matrices[bone_index].offset_matrix;
    }

    for (unsigned int i = 0; i < p_node->mNumChildren; i++) {
        readNodeHierarchy(p_animation_time, p_node->mChildren[i], global_transform);
    }
}

void Model::boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms) {
    aiMatrix4x4 identity_matrix;  // = mat4(1.0f);

    double time_in_ticks = time_in_sec * ticks_per_second;
    float animation_time = fmod(time_in_ticks, duration);

    readNodeHierarchy(animation_time, scene->mRootNode, identity_matrix);

    transforms.resize(m_num_bones);

    for (unsigned int i = 0; i < m_num_bones; i++) {
        transforms[i] = m_bone_matrices[i].final_world_transform;
    }
}

glm::mat4 Model::aiToGlm(aiMatrix4x4 ai_matr) {
    glm::mat4 result;
    result[0].x = ai_matr.a1;
    result[0].y = ai_matr.b1;
    result[0].z = ai_matr.c1;
    result[0].w = ai_matr.d1;

    result[1].x = ai_matr.a2;
    result[1].y = ai_matr.b2;
    result[1].z = ai_matr.c2;
    result[1].w = ai_matr.d2;

    result[2].x = ai_matr.a3;
    result[2].y = ai_matr.b3;
    result[2].z = ai_matr.c3;
    result[2].w = ai_matr.d3;

    result[3].x = ai_matr.a4;
    result[3].y = ai_matr.b4;
    result[3].z = ai_matr.c4;
    result[3].w = ai_matr.d4;

    return result;
}

aiQuaternion Model::nlerp(aiQuaternion a, aiQuaternion b, float blend) {
    // cout << a.w + a.x + a.y + a.z << endl;
    a.Normalize();
    b.Normalize();

    aiQuaternion result;
    float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    float one_minus_blend = 1.0f - blend;

    if (dot_product < 0.0f) {
        result.x = a.x * one_minus_blend + blend * -b.x;
        result.y = a.y * one_minus_blend + blend * -b.y;
        result.z = a.z * one_minus_blend + blend * -b.z;
        result.w = a.w * one_minus_blend + blend * -b.w;
    } else {
        result.x = a.x * one_minus_blend + blend * b.x;
        result.y = a.y * one_minus_blend + blend * b.y;
        result.z = a.z * one_minus_blend + blend * b.z;
        result.w = a.w * one_minus_blend + blend * b.w;
    }

    return result.Normalize();
}

}  // namespace gdp1
