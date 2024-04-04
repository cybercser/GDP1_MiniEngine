#pragma once

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

namespace gdp1 {

class GLMUtils {

public:
    static glm::mat4 aiMatrix4x4ToGlmMat4(const aiMatrix4x4& matrix);

};

}  // namespace gdp1
