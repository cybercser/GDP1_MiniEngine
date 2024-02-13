#include "collider.h"

#include "Core/game_object.h"
#include "Render/model.h"

namespace gdp1 {

#pragma region SphereCollider
Bounds SphereCollider::GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const {
    return Bounds{pos, glm::vec3{radius, radius, radius}};
}

Bounds SphereCollider::GetBounds(const glm::mat4& transMat) const {
    glm::vec3 pos = transMat[3];
    glm::vec3 scale = glm::vec3{glm::length(transMat[0]), glm::length(transMat[1]), glm::length(transMat[2])};
    return Bounds{pos, glm::vec3{radius, radius, radius} * scale};
}

Bounds SphereCollider::GetBounds() const {
    return Bounds{glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{radius, radius, radius}};
}
#pragma endregion

#if 0
#pragma region BoxCollider
Bounds BoxCollider::GetBounds(const glm::vec3& pos, const glm::vec3& lossyScale, const glm::quat& orient) const {
}

Bounds BoxCollider::GetBounds(const glm::mat4& transMat) const {
}

Bounds BoxCollider::GetBounds() const {
}
#pragma endregion
#endif

#pragma region MeshCollider
MeshCollider::MeshCollider(GameObject* pObject)
    : object(pObject) {
    centerOfMass = glm::vec3(0.0);

    // copy vertices
    Model* model = object->model;
    if (model == nullptr) {
        return;  // ignore empty game object
    }

    // copy mesh's transform so that the collider coincides with the mesh
    // combine all meshes into one
    // #TODO: simplified collider mesh (convex hull)
    glm::mat4 transMat = object->transform->WorldMatrix();
    unsigned int indexOffset = 0;
    for (auto& mesh : model->meshes) {
        for (auto& vertex : mesh.vertices) {
            ColliderVertex cv;
            cv.position = glm::vec3(glm::vec4(vertex.position, 1.0f) * transMat);
            cv.texCoords = vertex.texCoords;
            hull_.vertices.push_back(cv);
        }
        for (auto& index : mesh.indices) {
            hull_.indices.push_back(index + indexOffset);
        }
        indexOffset += mesh.vertices.size();
    }
    SetupMesh();

    // transform bounds
    glm::vec3 min = model->bounds.GetMin();
    glm::vec3 max = model->bounds.GetMax();
    glm::vec3 transformedMin = glm::vec3(transMat * glm::vec4(min, 1.0f));
    glm::vec3 transformedMax = glm::vec3(transMat * glm::vec4(max, 1.0f));

    this->bounds_.SetMinMax(transformedMin, transformedMax);
}

Bounds MeshCollider::GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const {
    glm::mat4 transMat =
        glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(orient) * glm::scale(glm::mat4(1.0f), scale);
    return this->GetBounds(transMat);
}

Bounds MeshCollider::GetBounds(const glm::mat4& transMat) const {
    glm::vec3 min = bounds_.GetMin();
    glm::vec3 max = bounds_.GetMax();
    glm::vec3 transformedMin = glm::vec3(transMat * glm::vec4(min, 1.0f));
    glm::vec3 transformedMax = glm::vec3(transMat * glm::vec4(max, 1.0f));
    Bounds result;
    result.SetMinMax(transformedMin, transformedMax);
    return result;
}

Bounds MeshCollider::GetBounds() const {
    return bounds_;
}

void MeshCollider::SetupMesh() {
    glGenVertexArrays(1, &hull_.VAO);
    glGenBuffers(1, &hull_.VBO);

    glBindVertexArray(hull_.VAO);

    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, hull_.VBO);
    glBufferData(GL_ARRAY_BUFFER, hull_.vertices.size() * sizeof(ColliderVertex), &hull_.vertices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColliderVertex), (void*)0);
    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ColliderVertex), (void*)offsetof(ColliderVertex, texCoords));

    glBindVertexArray(0);
}

#pragma endregion

}  // namespace gdp1
