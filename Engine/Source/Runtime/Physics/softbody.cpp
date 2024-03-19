#include "softbody.h"

#include "Render/mesh.h"
#include "Utils/transform_utils.h"
#include "Core/game_object.h"

#include <iostream>

#define MIN_FLOAT 1.192092896e-07f

DWORD WINAPI UpdateSoftBodyThread(LPVOID lpParameter);

namespace gdp1 {

SoftBodyParticle::SoftBodyParticle() {
    position = glm::vec3(0.0f);
    oldPosition = glm::vec3(0.0f);
    worldPosition = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);

    mass = 1.0;
    isPinned = false;
    go = nullptr;
    pVertex = nullptr;
}

SoftBody::SoftBody() {
    SoftBodyThreadInfo* tInfo = new SoftBodyThreadInfo();
    tInfo->body = this;
    tInfo->isAlive = true;
    tInfo->keepRunning = false;
    tInfo->timeStep = 0.02;
    tInfo->sleepTime = 0.1;

    // params = (void*)tInfo;
    // handle = CreateThread(NULL, 0, UpdateSoftBodyThread, params, 0, &(threadId));
}

void SoftBody::CreateParticles(Model* model, Transform* transform) {
    this->transform = transform;
    this->model = model;

    if (model == nullptr) {
        LOG_ERROR("ERROR: Expected model should not be null.");
        return;
    }

    if (model->meshes.empty()) {
        LOG_ERROR("ERROR: Expected game object had atleast one mesh.");
        return;
    }

    if (particles.size() > 0) return;

    for (unsigned int i = 0; i < model->meshes.size(); i++) {
        Mesh meshToCopy = model->meshes[i];

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

            vertex.position = meshToCopy.vertices[i].position;
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

        Mesh mesh = Mesh(vertices, indices, textures, bounds, true);

        size_t sizeVertices = mesh.vertices.size();

        this->particles.reserve(sizeVertices);

        // Create particles
        for (Vertex vertex : mesh.vertices) {
            SoftBodyParticle* pParticle = new SoftBodyParticle();
            glm::vec4 _vertex = glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1.0f);

            // std::cout << vertex.position.x << ", " << vertex.position.y << ", " << vertex.position.z << std::endl;

            glm::mat4 matTransform = glm::mat4(1.0f);

            matTransform = glm::translate(matTransform, transform->localPosition);
            matTransform *= glm::mat4_cast(transform->localRotation);
            matTransform = glm::scale(matTransform, transform->localScale);

            _vertex = matTransform * _vertex;

            pParticle->position = _vertex;
            pParticle->oldPosition = pParticle->position;
            pParticle->worldPosition = pParticle->position;
            pParticle->mass = this->particleMass;
            pParticle->pVertex = &vertex;

            particles.push_back(pParticle);
        }

        meshes.push_back(mesh);
    }

    for (unsigned int i = 0; i < particles.size(); i++) {
        SoftBodyParticle* particle = particles[i];
        particle->position = TransformUtils::LocalToWorldPoint(particle->position, transform->localPosition,
                                                               transform->localRotation, transform->localScale.x);
        particle->oldPosition = particle->position;
        particle->worldPosition = particle->position;
    }

    // Create springs
    for (unsigned int index = 0; index < meshes.size(); index++) {
        for (unsigned int i = 0; i < meshes[index].indices.size(); i += 3) {
            SoftBodyParticle* particleA = particles[meshes[index].indices[i]];
            SoftBodyParticle* particleB = particles[meshes[index].indices[i + 1]];
            SoftBodyParticle* particleC = particles[meshes[index].indices[i + 2]];

            CreateSpring(particleA, particleB);
            CreateSpring(particleB, particleC);
            CreateSpring(particleC, particleA);
        }
    }

    /*for (unsigned int i = 0; i < particles.size() - 1; i++) {
        SoftBodyParticle* particleA = particles[i];
        SoftBodyParticle* particleB = particles[i + 1];

        CreateSpring(particleA, particleB);
    }*/

    for (unsigned int i = 0; i < springs.size(); i += 3) {
        if (i + 3 < springs.size()) {
            SoftBodySpring* springA = springs[i];
            SoftBodySpring* springB = springs[i + 3];

            CreateSpring(springA->particleA, springB->particleA);
            CreateSpring(springA->particleB, springB->particleB);
        }
    }

    if (!particles.empty()) {
        SoftBodyParticle* firstParticle = particles.front();
        SoftBodyParticle* lastParticle = particles.back();
        CreateSpring(lastParticle, firstParticle);
    }
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

void SoftBody::CreateRandomSprings(int numOfSprings, float minDistance) {
    for (unsigned int count = 0; count != numOfSprings; count++) {
        // Assume the distance is OK
        bool bKeepLookingForParticles = false;

        do {
            // Assume the distance is OK
            bKeepLookingForParticles = false;

            unsigned int particleIndex1 = rand() % this->particles.size();
            unsigned int particleIndex2 = rand() % this->particles.size();

            SoftBodyParticle* pParticle1 = this->particles[particleIndex1];
            SoftBodyParticle* pParticle2 = this->particles[particleIndex2];
            float distBetween = glm::distance(pParticle1->position, pParticle2->position);

            // Distance OK?
            if (distBetween < minDistance) {
                bKeepLookingForParticles = true;
            } else {
                CreateSpring(pParticle1, pParticle2);
            }

        } while (bKeepLookingForParticles);
    }
}

void SoftBodySpring::Update(const float& springStrength, int iterations) {
    if (isActive) {
        for (int i = 0; i < iterations; i++) {
            if (particleB != nullptr && particleA != nullptr) {
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
    }

    if (go != nullptr) {
        go->transform->SetPosition(position);
    }
}

void SoftBodyParticle::ApplyForce(glm::vec3 force) { this->velocity += force * mass; }

void SoftBody::ApplyForce(const glm::vec3& force) {
    for (SoftBodyParticle* particle : particles) {
        particle->ApplyForce(force);
    }
}

void SoftBody::Draw(Shader* shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].UpdateVertexBuffers();
        meshes[i].Draw(shader);
    }
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
    for (unsigned int index = 0; index < meshes.size(); index++) {
        for (size_t i = 0; i < meshes[index].vertices.size(); i++) {
            glm::vec3 localPosition =
                TransformUtils::WorldToLocalPoint(this->particles[i]->position, transform->localPosition,
                                                  transform->localRotation, transform->localScale.x);

            meshes[index].vertices[i].position = localPosition;

            if (this->particles[i]->position.y < 0.0f) {
                this->particles[i]->position.y = 0.0f;
            }
        }
    }

    return;
}

void SoftBody::UpdateNormals() {
    for (unsigned int index = 0; index < meshes.size(); index++) {
        for (Vertex& vertex : meshes[index].vertices) {
            vertex.normal = glm::vec3(0.f);
        }

        for (unsigned int i = 0; i < meshes[index].indices.size(); i += 3) {
            unsigned int vertAIndex = meshes[index].indices[i];
            unsigned int vertBIndex = meshes[index].indices[i + 1];
            unsigned int vertCIndex = meshes[index].indices[i + 2];

            Vertex& vertexA = meshes[index].vertices[vertAIndex];
            Vertex& vertexB = meshes[index].vertices[vertBIndex];
            Vertex& vertexC = meshes[index].vertices[vertCIndex];

            glm::vec3 normal = glm::cross(vertexB.position - vertexA.position, vertexC.position - vertexA.position);

            vertexA.normal += normal;
            vertexB.normal += normal;
            vertexC.normal += normal;
        }

        // Normalize vertex normals
        for (Vertex& vertex : meshes[index].vertices) {
            vertex.normal = glm::normalize(vertex.normal);
        }
    }

    return;
}

void SoftBody::Simulate() {
    // SoftBodyThreadInfo* tInfo = (SoftBodyThreadInfo*) params;
    // tInfo->keepRunning = true;
}

}  // namespace gdp1