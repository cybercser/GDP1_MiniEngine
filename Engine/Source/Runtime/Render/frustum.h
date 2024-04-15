#pragma once

#include <common.h>

namespace gdp1 {

class GameObject;

class Frustum {
public:
    Frustum() = default;

    void Update(const glm::mat4& viewProjectionMatrix);
    bool IsBoxInFrustum(const glm::vec3& min, const glm::vec3& max);

    std::unordered_map<std::string, GameObject*> GetCulledObjects(
        std::unordered_map<std::string, GameObject*>& gameObjectMap);

public:
    glm::mat4 viewProjectionMatrix;

private:
    glm::vec4 planes[6];
};

}  // namespace gdp1
