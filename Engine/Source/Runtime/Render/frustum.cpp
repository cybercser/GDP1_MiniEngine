#include "frustum.h"

#include <Render/model.h>
#include <Core/game_object.h>

namespace gdp1 {

void Frustum::Update(const glm::mat4& viewProjectionMatrix) {
    this->viewProjectionMatrix = viewProjectionMatrix;

    // Left plane
    planes[0] = glm::vec4(viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0],
                          viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0],
                          viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0],
                          viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0]);

    // Right plane
    planes[1] = glm::vec4(viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0],
                          viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0],
                          viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0],
                          viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0]);

    // Bottom plane
    planes[2] = glm::vec4(viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1],
                          viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1],
                          viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1],
                          viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1]);

    // Top plane
    planes[3] = glm::vec4(viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1],
                          viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1],
                          viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1],
                          viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1]);

    // Near plane
    planes[4] = glm::vec4(viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2],
                          viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2],
                          viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2],
                          viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2]);

    // Far plane
    planes[5] = glm::vec4(viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2],
                          viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2],
                          viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2],
                          viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2]);
}

bool Frustum::IsBoxInFrustum(const glm::vec3& min, const glm::vec3& max) {
    for (int i = 0; i < 6; ++i) {
        if ((glm::dot(planes[i], glm::vec4(min.x, min.y, min.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(max.x, min.y, min.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(min.x, max.y, min.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(max.x, max.y, min.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(min.x, min.y, max.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(max.x, min.y, max.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(min.x, max.y, max.z, 1.0f)) < 0.0f) &&
            (glm::dot(planes[i], glm::vec4(max.x, max.y, max.z, 1.0f)) < 0.0f)) {
            return false;
        }
    }
    return true;
}

std::unordered_map<std::string, GameObject*> Frustum::GetCulledObjects(
    std::unordered_map<std::string, GameObject*>& gameObjectMap) {
    std::unordered_map<std::string, GameObject*> culledObjects;

 #pragma omp parallel for
    for (std::unordered_map<std::string, GameObject*>::iterator it = gameObjectMap.begin(); it != gameObjectMap.end();
         it++) {
        if (!it->second->visible) continue;

        Bounds bounds = it->second->model->bounds;
        bounds.TransformBounds(it->second->transform->WorldMatrix());

        if (IsBoxInFrustum(bounds.GetMin(), bounds.GetMax())) {
            culledObjects[it->first] = it->second;
        }
    }

    return culledObjects;
}

}  // namespace gdp1