#pragma once
#include <glad/glad.h>
#include <vector>

namespace gdp1 {

class EBO {

public:
    // ID reference of Elements Buffer Object
    GLuint ID;
    // Constructor that generates a Elements Buffer Object and links it to indices
    EBO(std::vector<GLuint>& indices, bool isDynamicBuffer);

    // Binds the EBO
    void Bind();
    // Unbinds the EBO
    void Unbind();
    // Deletes the EBO
    void Delete();
};

}  // namespace gdp1
