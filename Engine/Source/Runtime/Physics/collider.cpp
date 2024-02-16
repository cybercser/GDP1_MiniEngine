#include "collider.h"

#include "Core/core.h"
#include "Core/game_object.h"
#include "Render/model.h"

using namespace glm;

namespace gdp1 {

//******************************************************************************
// SPHERE COLLIDER
//******************************************************************************
#pragma region SphereCollider
Bounds SphereCollider::GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const {
    return Bounds{pos, vec3{radius, radius, radius} * 2.0f * scale};
}

Bounds SphereCollider::GetBounds(const glm::mat4& transMat) const {
    vec3 pos = transMat[3];
    vec3 scale = vec3{length(transMat[0]), length(transMat[1]), length(transMat[2])};
    return Bounds{pos, vec3{radius, radius, radius} * 2.0f * scale};
}

Bounds SphereCollider::GetBounds() const {
    GLCORE_ASSERT(gameObject, "GameObject must not be null");
    GLCORE_ASSERT(gameObject->transform, "GameObject must have a transform component");
    return GetBounds(gameObject->transform->WorldMatrix());
}
#pragma endregion

//******************************************************************************
// BOX COLLIDER
//******************************************************************************
#pragma region BoxCollider
Bounds BoxCollider::GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const {
    // calculate the transform matrix
    mat4 transMat = mat4(1.0f);
    transMat = translate(transMat, pos);
    transMat *= mat4(orient);
    transMat = glm::scale(transMat, scale);
    return GetBounds(transMat);
}

Bounds BoxCollider::GetBounds(const glm::mat4& transMat) const {
    vec3 min = center - size / 2.0f;
    vec3 max = center + size / 2.0f;
    vec3 transformedMin = vec3(transMat * vec4(min, 1.0f));
    vec3 transformedMax = vec3(transMat * vec4(max, 1.0f));

    return Bounds{transformedMin, transformedMax};
}

Bounds BoxCollider::GetBounds() const {
    GLCORE_ASSERT(gameObject, "GameObject must not be null");
    GLCORE_ASSERT(gameObject->transform, "GameObject must have a transform component");
    return GetBounds(gameObject->transform->WorldMatrix());
}
#pragma endregion

//******************************************************************************
// CAPSULE COLLIDER
//******************************************************************************
#pragma region CapsuleCollider
Bounds CapsuleCollider::GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const {
    // calculate the transform matrix
    mat4 transMat = mat4(1.0f);
    transMat = translate(transMat, pos);
    transMat *= mat4(orient);
    transMat = glm::scale(transMat, scale);
    return GetBounds(transMat);
}

Bounds CapsuleCollider::GetBounds(const glm::mat4& transMat) const {
    float size = GetHeight();
    vec3 min = center - size / 2.0f;
    vec3 max = center + size / 2.0f;
    vec3 transformedMin = vec3(transMat * vec4(min, 1.0f));
    vec3 transformedMax = vec3(transMat * vec4(max, 1.0f));

    return Bounds{transformedMin, transformedMax};
}

Bounds CapsuleCollider::GetBounds() const {
    GLCORE_ASSERT(gameObject, "GameObject must not be null");
    GLCORE_ASSERT(gameObject->transform, "GameObject must have a transform component");
    return GetBounds(gameObject->transform->WorldMatrix());
}
#pragma endregion

//******************************************************************************
// MESH COLLIDER
//******************************************************************************
#pragma region MeshCollider
MeshCollider::MeshCollider(GameObject* pObject) {
    center = glm::vec3(0.0);
    gameObject = pObject;

    // copy vertices
    Model* model = gameObject->model;
    if (model == nullptr) {
        return;  // ignore empty game object
    }

    // copy mesh's transform so that the collider coincides with the mesh
    // combine all meshes into one
    // #TODO: simplify collider mesh (convex hull)
    unsigned int indexOffset = 0;
    m_Mesh.vertices.reserve(model->GetVertexCount());
    for (auto& mesh : model->meshes) {
        for (auto& vertex : mesh.vertices) {
            ColliderVertex cv;
            cv.position = vertex.position;
            cv.texCoords = vertex.texCoords;
            cv.normal = vertex.normal;
            m_Mesh.vertices.push_back(cv);
        }

        for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
            ColliderTriangle ct;
            ct.indices[0] = mesh.indices[i] + indexOffset;
            ct.indices[1] = mesh.indices[i + 1] + indexOffset;
            ct.indices[2] = mesh.indices[i + 2] + indexOffset;
            m_Mesh.triangles.push_back(ct);
        }
        indexOffset += mesh.vertices.size();
    }

    this->m_OriginalBounds = model->bounds;
}

Bounds MeshCollider::GetBounds(const glm::vec3& pos, const glm::vec3& scale, const glm::quat& orient) const {
    mat4 transMat = mat4(1.0f);

    transMat = translate(transMat, pos);
    transMat *= mat4(orient);
    transMat = glm::scale(transMat, scale);

    return GetBounds(transMat);
}

Bounds MeshCollider::GetBounds(const glm::mat4& transMat) const {
    vec3 min = m_OriginalBounds.GetMin();
    vec3 max = m_OriginalBounds.GetMax();
    vec3 transformedMin = vec3(transMat * vec4(min, 1.0f));
    vec3 transformedMax = vec3(transMat * vec4(max, 1.0f));
    return Bounds{transformedMin, transformedMax};
}

Bounds MeshCollider::GetBounds() const {
    GLCORE_ASSERT(gameObject, "GameObject must not be null");
    GLCORE_ASSERT(gameObject->transform, "GameObject must have a transform component");
    return GetBounds(gameObject->transform->WorldMatrix());
}

#pragma endregion

}  // namespace gdp1
