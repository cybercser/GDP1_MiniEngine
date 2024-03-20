#pragma once

#include <common.h>
#include "Render/model.h"
#include "Render/shader.h"
#include "Core/transform.h"
#include "Core/cs_runner.h"
#include "collider.h"

namespace gdp1 {

class GameObject;
class SoftBody;

struct SoftBodyThreadInfo {
    SoftBody* body = nullptr;
    double timeStep = 0.0;
    DWORD sleepTime = 0;
    bool isAlive = true;
    bool keepRunning = true;
};

struct SoftBodyParticle {

    SoftBodyParticle();

    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 worldPosition;

    glm::vec3 velocity;
    glm::vec3 acceleration;

    float mass;
    bool isPinned;

    GameObject* go;

    Vertex* pVertex;

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

    bool isActive = true;

    void Update(const float& springStrength, int iterations);
};

class SoftBody: public CSRunner {
public:
    SoftBody();
    ~SoftBody();

    float springStrength = 0.f;
    float particleMass = 0.f;

    int iterations = 0;

    void CreateParticles(Model* model, Transform* transform);

    void CreateSpring(SoftBodyParticle* particleA, SoftBodyParticle* particleB);

    void CreateRandomSprings(int numOfSprings, float minDistance);

    void ApplyForce(const glm::vec3& force);
    void Update(float deltaTime);

    void UpdatePositions(float deltaTime);
    void UpdateNormals();

    void Draw(Shader* shader);

    Model* model;
    Collider* collider;

    std::vector<Mesh> meshes;

    std::vector<SoftBodyParticle*> particles{};
    std::vector<SoftBodySpring*> springs{};

    Transform* transform;

};

}  // namespace gdp1
