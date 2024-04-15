#include "skybox.h"

#include "Resource/texture.h"
#include "shader.h"

#include <Core/application.h>
#include <GLFW/glfw3.h>

namespace gdp1 {

Skybox::Skybox(const std::vector<std::string>& faces, float size)
    : texId_(0)
    , VAO_(0)
    , VBO_(0)
    , EBO_(0) {
    lastTime = glfwGetTime();
    LoadCubemap(faces);
    InitSkyboxVAO(size);
}

Skybox::~Skybox() {
    if (VAO_ != 0) {
        glDeleteVertexArrays(1, &VAO_);
        VAO_ = 0;
    }
    if (VBO_ != 0) {
        glDeleteBuffers(1, &VBO_);
        VBO_ = 0;
    }
    if (EBO_ != 0) {
        glDeleteBuffers(1, &EBO_);
        EBO_ = 0;
    }
    if (texId_ != 0) {
        glDeleteTextures(1, &texId_);
        texId_ = 0;
    }

    indices_.clear();
    vertices_.clear();
}

void Skybox::Draw(Shader* shader, const glm::mat4& viewMat, const glm::mat4 projMat) {
    if (VAO_ == 0) return;

    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);

    double currentTime = glfwGetTime();

    shader->Use();
    glm::mat4 view = glm::mat4(glm::mat3(viewMat));  // remove translation from the view matrix
    shader->SetUniform("u_View", view);
    shader->SetUniform("u_Proj", projMat);
    //shader->SetUniform("u_Time", float(currentTime - lastTime));
    
    lastTime = currentTime;

    glBindVertexArray(VAO_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texId_);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glCullFace(GL_BACK);

    Application::drawCalls++;
}

/*
       +-------+
       |  +Y   |
       |  top  |
+------+-------+-------+------+
|  -X  |  +Z   |  +X   |  -Z  |
| left | front | right | back |
+------+-------+-------+------+
       |  -Y   |
       | bottom|
       +-------+
*/
// the order of faces:
// +X, -X, +Y, -Y, +Z, -Z
void Skybox::LoadCubemap(const std::vector<std::string>& faces) { texId_ = Texture::LoadCubeMap(faces); }

/*
   .+------2
 .'  T   .'|
+------3'  |
|      | R |
|   F  |   1
|      | .'
+------0'

   .7------+
 .' |      |
6   | BACK |
| L |      |
|  .4------+
|.'  BTM .'
5------+'

winding order: CCW
    3-----2
    |     |
    |     |
    0-----1
*/

void Skybox::InitSkyboxVAO(float size) {
    float hs = size / 2.0f;  // half size
    // we only need positions
    vertices_ = {
        hs,  -hs, hs,   // 0
        hs,  -hs, -hs,  // 1
        hs,  hs,  -hs,  // 2
        hs,  hs,  hs,   // 3
        -hs, -hs, -hs,  // 4
        -hs, -hs, hs,   // 5
        -hs, hs,  hs,   // 6
        -hs, hs,  -hs,  // 7
    };

    // indices
    indices_ = {
        // Right
        0,
        1,
        2,
        2,
        3,
        0,
        // Left
        4,
        5,
        6,
        6,
        7,
        4,
        // Top
        6,
        3,
        2,
        2,
        7,
        6,
        // Bottom
        4,
        1,
        0,
        0,
        5,
        4,
        // Front
        5,
        0,
        3,
        3,
        6,
        5,
        // Back
        1,
        4,
        7,
        7,
        2,
        1,
    };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);
    glGenBuffers(1, &EBO_);

    // bind VAO
    glBindVertexArray(VAO_);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(GLfloat), vertices_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), indices_.data(), GL_STATIC_DRAW);

    // set position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // unbind VAO
    glBindVertexArray(0);
}

}  // namespace gdp1
