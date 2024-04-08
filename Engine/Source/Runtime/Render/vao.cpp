#include "vao.h"

namespace gdp1 {

// Constructor that generates a VAO ID
VAO::VAO() { glGenVertexArrays(1, &ID); }

// Links a VBO Attribute such as a position or color to the VAO
void VAO::LinkAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    glEnableVertexAttribArray(layout);
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
}

void VAO::Link_iAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    glEnableVertexAttribArray(layout);
    glVertexAttribIPointer(layout, numComponents, type, stride, offset);
}

// Binds the VAO
void VAO::Bind() { glBindVertexArray(ID); }

// Unbinds the VAO
void VAO::Unbind() { glBindVertexArray(0); }

// Deletes the VAO
void VAO::Delete() { glDeleteVertexArrays(1, &ID); }

}  // namespace gdp1