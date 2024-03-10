#pragma once

#include <glm/glm.hpp>

namespace gdp1 {

struct PNTVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct IndexedTriangle {
    unsigned int indices[3];
};

struct PosTriangle {
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

}  // namespace gdp1
