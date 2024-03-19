#pragma once

#include <common.h>
#include "Render/model.h"
#include "Render/shader.h"
#include "Core/transform.h"
#include "collider.h"

namespace gdp1 {

class GameObject;
class SoftBody;

struct SoftBodyThreadInfo {
    SoftBody* body;
    double timeStep = 0.0;
    DWORD sleepTime;
    bool isAlive = true;
    bool keepRunning = true;
};

struct SoftBodyParticle {

    SoftBodyParticle();

    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 worldPosition;

    glm::vec3 velocity;

    glm::vec3 acceleration = glm::vec3(0.f, -9.8f, 0.f);

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

    void Update(const float& springStregth, int iterations);
};

class SoftBody {
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

    void Simulate();

    void Draw(Shader* shader);

    Model* model;
    Collider* collider;

    std::vector<Mesh> meshes;

    //DWORD threadId;
    //HANDLE handle;
    //void* params;

    std::vector<SoftBodyParticle*> particles{};
    std::vector<SoftBodySpring*> springs{};

    Transform* transform;

};

}  // namespace gdp1
