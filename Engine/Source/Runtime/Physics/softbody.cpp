#include "softbody.h"

#include "Render/mesh.h"
#include "Utils/transform_utils.h"

#define MIN_FLOAT 1.192092896e-07f

namespace gdp1 {

SoftBody::SoftBody(Model* model, Transform* transform) {
    // this->transform = transform;

    if (model == nullptr) {
        LOG_ERROR("ERROR: Expected model should not be null.");
        return;
    }

    if (model->meshes.empty()) {
        LOG_ERROR("ERROR: Expected game object had atleast one mesh.");
        return;
    }

    if (particles.size() > 0) return;

    Mesh meshToCopy = model->meshes[0];
    mesh = meshToCopy;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureInfo> textures;
    Bounds bounds;

    vertices.resize(meshToCopy.vertices.size());
    indices.resize(meshToCopy.indices.size());
    bounds = meshToCopy.bounds;

    for (unsigned int i = 0; i < meshToCopy.vertices.size(); i++) {
        Vertex& vertex = vertices[i];
        vertex.SetBoneDefaults();

        glm::vec4 _vertex = glm::vec4(meshToCopy.vertices[i].position.x, meshToCopy.vertices[i].position.y,
                                      meshToCopy.vertices[i].position.z, 1.0f);

        _vertex = transform->WorldMatrix() * _vertex;

        vertex.position = glm::vec3(_vertex.x, _vertex.y, _vertex.z);
        vertex.normal = meshToCopy.vertices[i].normal;
        vertex.texCoords = meshToCopy.vertices[i].texCoords;
        vertex.tangent = meshToCopy.vertices[i].tangent;
        vertex.bitangent = meshToCopy.vertices[i].bitangent;

        for (int index = 0; index < MAX_BONE_INFLUENCE; index++) {
            vertex.AddBoneData(meshToCopy.vertices[i].boneIDs[index], meshToCopy.vertices[i].weights[index]);
        }
    }

    for (int i = 0; i < meshToCopy.indices.size(); i++) {
        indices[i] = meshToCopy.indices[i];
    }

    for (int i = 0; i < meshToCopy.textures.size(); i++) {
        TextureInfo& texture = textures[i];

        texture.id = meshToCopy.textures[i].id;
        texture.path = meshToCopy.textures[i].path;
        texture.type = meshToCopy.textures[i].type;
    }

    mesh = Mesh(vertices, indices, textures, bounds, true);

    size_t sizeVertices = mesh.vertices.size();

    this->particles.reserve(sizeVertices);

    // Create particles
    for (const Vertex& vertex : mesh.vertices) {
        SoftBodyParticle* pParticle = new SoftBodyParticle();
        pParticle->position = vertex.position;
        pParticle->oldPosition = pParticle->position;
        pParticle->worldPosition = pParticle->position;
        particles.push_back(pParticle);
    }

    particles[0]->isPinned = true;

    // Create constraints
    for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
        SoftBodyParticle* particleA = particles[mesh.indices[i]];
        SoftBodyParticle* particleB = particles[mesh.indices[i + 1]];
        SoftBodyParticle* particleC = particles[mesh.indices[i + 2]];

        CreateSpring(particleA, particleB);
        CreateSpring(particleB, particleC);
        CreateSpring(particleC, particleA);
    }

    CreateRandomSprings(3000);
}

SoftBody::~SoftBody() {
    // Clean up particles
    for (SoftBodyParticle* particle : particles) {
        delete particle;
    }
    particles.clear();

    // Clean up springs
    for (SoftBodySpring* spring : springs) {
        delete spring;
    }
    springs.clear();
}

void SoftBody::CreateSpring(SoftBodyParticle* particleA, SoftBodyParticle* particleB) {
    SoftBodySpring* pSpring = new SoftBodySpring();
    pSpring->restLength = glm::distance(particleA->position, particleB->position);
    pSpring->particleA = particleA;
    pSpring->particleB = particleB;
    springs.push_back(pSpring);
}

void SoftBody::CreateRandomSprings(int numOfSprings) {
    for (unsigned int count = 0; count != numOfSprings; count++) {
        // Assume the distance is OK
        bool bKeepLookingForParticles = false;

        do {
            // Assume the distance is OK
            bKeepLookingForParticles = false;

            // Pick two random vertices
            // NOTE: Here, rand() might not be great because there's usually
            // only about 32,000 possible integer values...
            // Meaning that if your are chosing from something LARGER than
            // around 32,000, you'll miss a bunch of values.
            // HOWEVER, you could also multiply rand() by itself
            unsigned int particleIndex1 = rand() % this->particles.size();
            unsigned int particleIndex2 = rand() % this->particles.size();

            SoftBodyParticle* pParticle1 = this->particles[particleIndex1];
            SoftBodyParticle* pParticle2 = this->particles[particleIndex2];
            float distBetween = glm::distance(pParticle1->position, pParticle2->position);

            // Distance OK?
            if (distBetween < 1.0f) {
                bKeepLookingForParticles = true;
            } else {
                CreateSpring(pParticle1, pParticle2);
            }

        } while (bKeepLookingForParticles);
    }
}

void SoftBodySpring::Update(const float& springStrength, int iterations) {
    for (int i = 0; i < iterations; i++) {
        glm::vec3 delta = particleB->position - particleA->position;
        float deltaLength = glm::length(delta);
        if (deltaLength <= MIN_FLOAT) {
            return;
        }

        float diffA = (deltaLength - restLength);
        float diff = diffA / deltaLength;

        glm::vec3 deltaPos = delta * 0.5f * diff * springStrength;

        if (!particleA->isPinned) particleA->position += deltaPos;
        if (!particleB->isPinned) particleB->position -= deltaPos;
    }
}

void SoftBodyParticle::Update(float deltaTime) {
    if (!isPinned) {
        glm::vec3 currentPos = this->position;
        glm::vec3 oldPos = this->oldPosition;

        //position += (currentPos - oldPos) + (velocity * static_cast<float>(deltaTime * deltaTime));
        //oldPosition = currentPos;

        // float deltaTimeSq = static_cast<float>(deltaTime * deltaTime);
        // glm::vec3 accelerationTerm = glm::vec3(0.f, -2.0f, 0.f) * deltaTimeSq;
        glm::vec3 newPos = currentPos;
        newPos.y -= 0.1;

        this->oldPosition = currentPos;
        this->position = newPos;

        /*glm::vec3 vel = this->position - this->oldPosition;
        this->oldPosition = this->position;

        glm::vec3 acc = glm::vec3(0.f, -0.5f, 0.f) / this->mass;

        this->position = vel + acc * deltaTime * deltaTime;*/
    }
}

void SoftBodyParticle::ApplyForce(glm::vec3 force) { this->velocity += force * mass; }

void SoftBody::ApplyForce(const glm::vec3& force) {
    for (SoftBodyParticle* particle : particles) {
        particle->ApplyForce(force);
    }
}

void SoftBody::Draw(Shader* shader) {
    mesh.UpdateVertexBuffers();
    mesh.Draw(shader);
}

void SoftBody::Update(float deltaTime) {
    for (SoftBodyParticle* particle : particles) {
        particle->Update(deltaTime);
    }

    for (SoftBodySpring* spring : springs) {
        spring->Update(springStrength, iterations);
    }

    UpdatePositions(deltaTime);
    UpdateNormals();

    return;
}

void SoftBody::UpdatePositions(float deltaTime) {
    size_t vecSize = this->particles.size();
    for (size_t i = 0; i < vecSize; i++) {
        mesh.vertices[i].position = this->particles[i]->position;

        /*if (this->particles[i]->position.y < 0.0f) {
            this->particles[i]->position.y = 0.0f;
        }*/
    }

    return;
}

void SoftBody::UpdateNormals() {
    for (Vertex& vertex : mesh.vertices) {
        vertex.normal = glm::vec3(0.f);
    }

    for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
        unsigned int vertAIndex = mesh.indices[i];
        unsigned int vertBIndex = mesh.indices[i + 1];
        unsigned int vertCIndex = mesh.indices[i + 2];

        Vertex& vertexA = mesh.vertices[vertAIndex];
        Vertex& vertexB = mesh.vertices[vertBIndex];
        Vertex& vertexC = mesh.vertices[vertCIndex];

        glm::vec3 normal = glm::cross(vertexB.position - vertexA.position, vertexC.position - vertexA.position);

        vertexA.normal += normal;
        vertexB.normal += normal;
        vertexC.normal += normal;
    }

    // Normalize vertex normals
    for (Vertex& vertex : mesh.vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }

    return;
}

}  // namespace gdp1