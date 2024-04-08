#include "mesh.h"

#include <GLFW/glfw3.h>
#include <Core/application.h>

#define ADD_LINE(a, b)          \
    boundsIndices.push_back(a); \
    boundsIndices.push_back(b);

namespace gdp1 {

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures,
           const Bounds& bounds, bool isDynamicBuffer) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    this->bounds = bounds;
    this->isDynamicBuffer = isDynamicBuffer;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    SetupMesh();

    SetupDebugData();
}

void Mesh::DrawTextures(Shader* shader) {
    // bind appropriate textures
    unsigned int diffuseUnit = 1;
    unsigned int specularUnit = 1;
    unsigned int normalUnit = 1;
    unsigned int heightUnit = 1;
    unsigned int opacityUnit = 1;
    for (int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);  // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") number = std::to_string(diffuseUnit++);
        // else if (name == "texture_specular")
        //	number = std::to_string(specularUnit++);
        else if (name == "texture_normal")
            number = std::to_string(normalUnit++);
        else if (name == "texture_height")
            number = std::to_string(heightUnit++);
        else if (name == "texture_opacity")
            number = std::to_string(opacityUnit++);

        // now set the sampler to the correct texture unit
        if (diffuseUnit < 2 && specularUnit < 2 && normalUnit < 2 && heightUnit < 2 &&
            opacityUnit < 2) {  // only deal with the first unit of each type
            std::string uniformName = "u_Material." + name + number;
            shader->SetUniform(uniformName.c_str(), i);
        }

        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
}

void Mesh::Draw(Shader* shader) {
    DrawTextures(shader);

    if (_VAO.ID == 93) {
        bool breakP = true;
    }

    // draw mesh
    glBindVertexArray(_VAO.ID);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, numInstances);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
    Application::drawCalls++;
}

void Mesh::UpdateVertexBuffers() {
    _VBO.UpdateVertexBuffers();
    Application::drawCalls++;
}

void Mesh::SetupInstancing(std::vector<glm::mat4>& instanceMatrix) {
    _VAO.Bind();

    _instanceVBO = VBO();
    _instanceVBO.Bind();
    _instanceVBO.BindData(instanceMatrix, isDynamicBuffer);
    
    _VAO.LinkAttrib(7, 4, GL_FLOAT, sizeof(glm::mat4), (void*)0);
    _VAO.LinkAttrib(8, 4, GL_FLOAT, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    _VAO.LinkAttrib(9, 4, GL_FLOAT, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    _VAO.LinkAttrib(10, 4, GL_FLOAT, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    // Update the matrix for every 1 instance
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glVertexAttribDivisor(10, 1);

    _instanceVBO.Unbind();
    numInstances = instanceMatrix.size();
}

void Mesh::ResetInstancing() {
    if (numInstances > 1 && _instanceVBO.ID != 0) {
        _instanceVBO.Delete();
    }

    numInstances = 1;
    return;
}

void Mesh::DrawDebug(Shader* shader) {
    // draw bounds
    glBindVertexArray(debugVAO);
    glDrawElements(GL_LINES, static_cast<unsigned int>(boundsIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    Application::drawCalls++;
}

 void Mesh::SetupMesh() {
     // create buffers/arrays
     _VAO.Bind();

     _VBO.Bind();
     _VBO.BindData(vertices, isDynamicBuffer);

     EBO _EBO(indices, isDynamicBuffer);

     _VAO.LinkAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
     _VAO.LinkAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
     _VAO.LinkAttrib(2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
     _VAO.LinkAttrib(3, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
     _VAO.LinkAttrib(4, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
     _VAO.Link_iAttrib(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));
     _VAO.LinkAttrib(6, 4, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, weights));

     // Unbind all to prevent accidentally modifying them
     _VAO.Unbind();
     _VBO.Unbind();
     _EBO.Unbind();

     _VBO.Delete();
     _EBO.Delete();
 }

void Mesh::SetupDebugData() {
    // calculate the 8 vertices of the bounding box
    // Y-up Z-forward X-right
    /*
   .2------6
 .'      .'|
 3------7'  |
|      |   |
|      |   4
|      | .'
 1------5'

   .2------6
 .' |      |
 3   |      |
|   |      |
|  .0------4
|.'      .'
 1------5'
    */

    glm::vec3 c = bounds.GetCenter();
    glm::vec3 e = bounds.GetExtents();
    boundsVertices.push_back(c + glm::vec3(-e.x, -e.y, -e.z));  // 0
    boundsVertices.push_back(c + glm::vec3(-e.x, -e.y, e.z));   // 1
    boundsVertices.push_back(c + glm::vec3(-e.x, e.y, -e.z));   // 2
    boundsVertices.push_back(c + glm::vec3(-e.x, e.y, e.z));    // 3
    boundsVertices.push_back(c + glm::vec3(e.x, -e.y, -e.z));   // 4
    boundsVertices.push_back(c + glm::vec3(e.x, -e.y, e.z));    // 5
    boundsVertices.push_back(c + glm::vec3(e.x, e.y, -e.z));    // 6
    boundsVertices.push_back(c + glm::vec3(e.x, e.y, e.z));     // 7

    // calculate the indices of the bounding box
    ADD_LINE(0, 1);
    ADD_LINE(0, 2);
    ADD_LINE(0, 4);
    ADD_LINE(1, 3);
    ADD_LINE(1, 5);
    ADD_LINE(2, 3);
    ADD_LINE(2, 6);
    ADD_LINE(3, 7);
    ADD_LINE(4, 5);
    ADD_LINE(4, 6);
    ADD_LINE(5, 7);
    ADD_LINE(6, 7);

    // create buffers/arrays
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);
    glGenBuffers(1, &debugEBO);

    glBindVertexArray(debugVAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, boundsVertices.size() * sizeof(glm::vec3), &boundsVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, boundsIndices.size() * sizeof(unsigned int), &boundsIndices[0],
                 GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void Vertex::SetBoneDefaults() {
    for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        boneIDs[i] = -1;
        weights[i] = 0.f;
    }
}

void Vertex::AddBoneData(unsigned int bone_id, float weight) {
    for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (weights[i] == 0.0) {
            boneIDs[i] = bone_id;
            weights[i] = weight;
            return;
        }
    }
}

}  // namespace gdp1
