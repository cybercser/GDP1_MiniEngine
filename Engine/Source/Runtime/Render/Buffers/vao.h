#pragma once

#include <glad/glad.h>
#include "VBO.h"

namespace gdp1 {

class VAO {

public:
    // ID reference for the Vertex Array Object
    GLuint ID;
    // Constructor that generates a VAO ID
    VAO();

    // Links a VBO Attribute such as a position or color to the VAO
    void LinkAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
    void Link_iAttrib(GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

    // Generate the VAO
    void Generate();
    
    // Binds the VAO
    void Bind();

    // Unbinds the VAO
    void Unbind();
    
    // Deletes the VAO
    void Delete();

};

}  // namespace gdp1
