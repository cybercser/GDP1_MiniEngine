#include "softbody.h"

#include "Render/mesh.h"
#include "Utils/transform_utils.h"
#include "Core/game_object.h"

#include <iostream>

#define MIN_FLOAT 1.192092896e-07f

namespace gdp1 {

SoftBodyParticle::SoftBodyParticle() {
    position = glm::vec3(0.0f);
    oldPosition = glm::vec3(0.0f);
    worldPosition = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);

    mass = 1.0;
    isPinned = false;
    model = nullptr;
    go = nullptr;
}

SoftBody::SoftBody() {}

void SoftBody::CreateParticles(Model* model, Transform* transform) {
    this->transform = transform;

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
    textures.resize(meshToCopy.textures.size());
    bounds = meshToCopy.bounds;

    for (unsigned int i = 0; i < meshToCopy.vertices.size(); i++) {
        Vertex& vertex = vertices[i];
        vertex.SetBoneDefaults();

        /*glm::vec4 _vertex = glm::vec4(meshToCopy.vertices[i].position.x, meshToCopy.vertices[i].position.y,
                                      meshToCopy.vertices[i].position.z, 1.0f);

        std::cout << i << ": " << _vertex.x << ", " << _vertex.y << ", " << _vertex.z << std::endl;

        glm::mat4 matTransform = glm::mat4(1.0f);

        matTransform = glm::translate(matTransform, transform->localPosition);
        matTransform *= glm::mat4_cast(transform->localRotation);
        matTransform = glm::scale(matTransform, transform->localScale);*/

        //_vertex = transform->WorldToLocalMatrix() * _vertex;
        //_vertex = matTransform * _vertex;

        vertex.position = meshToCopy.vertices[i].position;  // glm::vec3(_vertex.x, _vertex.y, _vertex.z);
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
        glm::vec4 _vertex = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1.0f);

        glm::mat4 matTransform = glm::mat4(1.0f);

        matTransform = glm::translate(matTransform, transform->localPosition);
        matTransform *= glm::mat4_cast(transform->localRotation);
        matTransform = glm::scale(matTransform, transform->localScale);

        _vertex = matTransform * _vertex;

        pParticle->position = _vertex;
        pParticle->oldPosition = pParticle->position;
        pParticle->worldPosition = pParticle->position;
        particles.push_back(pParticle);
    }

    for (unsigned int i = 0; i < particles.size(); i++) {
        SoftBodyParticle* particle = particles[i];
        particle->position = TransformUtils::LocalToWorldPoint(particle->position, transform->localPosition,
                                                               transform->localRotation, transform->localScale.x);
        particle->oldPosition = particle->position;
        particle->worldPosition = particle->position;
    }

    //particles[0]->isPinned = true;

    // Create springs
    for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
        SoftBodyParticle* particleA = particles[mesh.indices[i]];
        SoftBodyParticle* particleB = particles[mesh.indices[i + 1]];
        SoftBodyParticle* particleC = particles[mesh.indices[i + 2]];

        CreateSpring(particleA, particleB);
        CreateSpring(particleB, particleC);
        CreateSpring(particleC, particleA);
    }

    for (unsigned int i = 0; i < particles.size() - 1; i++) {
        SoftBodyParticle* particleA = particles[i];
        SoftBodyParticle* particleB = particles[i + 1];

        CreateSpring(particleA, particleB);
    }

    //for (unsigned int i = 0; i < springs.size(); i += 3) {
    //    if (i + 3 < springs.size()) {
    //        SoftBodySpring* springA = springs[i];
    //        SoftBodySpring* springB = springs[i + 3];

    //        CreateSpring(springA->particleA, springB->particleA);
    //        CreateSpring(springA->particleB, springB->particleB);
    //    }
    //}
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
            if (distBetween < 0.5f) {
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
    if (deltaTime > 1.0f) deltaTime = 0.01f;

    if (!isPinned) {
        glm::vec3 currentPos = this->position;
        glm::vec3 oldPos = this->oldPosition;

        float deltaTimeSq = static_cast<float>(deltaTime * deltaTime);

        position += (currentPos - oldPos) + (velocity * deltaTimeSq);
        oldPosition = currentPos;

        if (go != nullptr) {
            go->transform->SetPosition(position);
        }
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
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        glm::vec3 localPosition = TransformUtils::WorldToLocalPoint(
            this->particles[i]->position, transform->localPosition, transform->localRotation, transform->localScale.x);

        mesh.vertices[i].position = localPosition;

        if (this->particles[i]->position.y < 0.0f) {
            this->particles[i]->position.y = 0.0f;
        }
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