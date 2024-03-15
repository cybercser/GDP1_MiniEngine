#pragma once

#include <common.h>
#include "Render/model.h"
#include "Render/shader.h"
#include "Core/transform.h"

namespace gdp1 {

class GameObject;

struct SoftBodyParticle {

    SoftBodyParticle();

    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 worldPosition;

    glm::vec3 velocity;

    glm::vec3 acceleration = glm::vec3(0.f, -9.8f, 0.f);

    float mass;
    bool isPinned;

    Model* model;
    GameObject* go;

    void Update(float deltaTime);
    void ApplyForce(glm::vec3 force);
};

struct SoftBodySpring {
    SoftBodySpring() {
        restLength = 0.0f;
        particleA = nullptr;
        particleB = nullptr;
    }

    float restLength;

    SoftBodyParticle* particleA;
    SoftBodyParticle* particleB;

    void Update(const float& springStregth, int iterations);
};

class SoftBody {
public:
    SoftBody();
    ~SoftBody();

    float springStrength = 1.0f;
    float particleMass = 1.0f;

    int iterations = 1;

    void CreateParticles(Model* model, Transform* transform);

    void CreateSpring(SoftBodyParticle* particleA, SoftBodyParticle* particleB);

    void CreateRandomSprings(int numOfSprings);

    void ApplyForce(const glm::vec3& force);
    void Update(float deltaTime);

    void UpdatePositions(float deltaTime);
    void UpdateNormals();

    void Draw(Shader* shader);

    Mesh mesh;

    std::vector<SoftBodyParticle*> particles{};
    std::vector<SoftBodySpring*> springs{};

    Transform* transform;

};

}  // namespace gdp1
