#include "ebo.h"

namespace gdp1 {

EBO::EBO(std::vector<unsigned int>& indices, bool isDynamicBuffer) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0],
                 isDynamicBuffer ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

// Binds the EBO
void EBO::Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }

// Unbinds the EBO
void EBO::Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

// Deletes the EBO
void EBO::Delete() { glDeleteBuffers(1, &ID); }

}
