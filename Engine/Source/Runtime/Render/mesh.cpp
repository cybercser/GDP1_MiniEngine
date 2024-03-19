#include "mesh.h"

#define ADD_LINE(a, b)          \
    boundsIndices.push_back(a); \
    boundsIndices.push_back(b);

namespace gdp1 {

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureInfo> textures,
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

void Mesh::Draw(Shader* shader) {
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

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    //glActiveTexture(GL_TEXTURE0);
}

void Mesh::UpdateVertexBuffers() {
    // Draw
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vertices.size(), (GLvoid*)&vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::DrawDebug(Shader* shader) {
    // draw bounds
    glBindVertexArray(debugVAO);
    glDrawElements(GL_LINES, static_cast<unsigned int>(boundsIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::SetupMesh() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2
    // array which again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glBindVertexArray(0);
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
