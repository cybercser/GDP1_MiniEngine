#include "vbo.h"

namespace gdp1 {

VBO::VBO() { glGenBuffers(1, &ID); }

void VBO::BindData(std::vector<Vertex>& vertices, bool isDynamicBuffer) {
    this->vertices = vertices;
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
                 isDynamicBuffer ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void VBO::BindData(std::vector<glm::mat4>& mat4s, bool isDynamicBuffer) {
    glBufferData(GL_ARRAY_BUFFER, mat4s.size() * sizeof(glm::mat4), &mat4s[0],
                 isDynamicBuffer ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void VBO::UpdateVertexBuffers() {
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), (GLvoid*)&vertices[0]);
}

// Binds the VBO
void VBO::Bind() { glBindBuffer(GL_ARRAY_BUFFER, ID); }

// Unbinds the VBO
void VBO::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

// Deletes the VBO
void VBO::Delete() { glDeleteBuffers(1, &ID); }

}  // namespace gdp1