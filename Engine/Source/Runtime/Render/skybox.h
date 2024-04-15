#pragma once

#include "common.h"

#include <glad/glad.h>

namespace gdp1 {

// forward declaration
class Shader;

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces, float size);
    ~Skybox();

    inline unsigned int GetCubemapTextureId() const { return texId_; }

    void Draw(Shader* shader, const glm::mat4& viewMat, const glm::mat4 projMat);

private:
    void LoadCubemap(const std::vector<std::string>& faces);
    void InitSkyboxVAO(float size);

private:
    unsigned int texId_;
    unsigned int VAO_, VBO_, EBO_;

    double lastTime = 0.0f;

    std::vector<GLfloat> vertices_;
    std::vector<GLuint> indices_;
};

}  // namespace gdp1
