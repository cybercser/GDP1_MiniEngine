#pragma once

#include <common.h>
#include "Render/model.h"
#include "Render/shader.h"
#include "Core/transform.h"

namespace gdp1 {

struct SoftBodyParticle {
    SoftBodyParticle() {
        position = glm::vec3(0.0f);
        oldPosition = glm::vec3(0.0f);
        worldPosition = glm::vec3(0.0f);
        velocity = glm::vec3(0.0f);

        mass = 1.0;
        isPinned = false;
    }

    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 worldPosition;

    glm::vec3 velocity;

    float mass;
    bool isPinned;

    Vertex* vertex;

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
    SoftBody(Model* model, Transform* transform);
    ~SoftBody();

    float springStrength = 1.0f;
    float particleMass = 1.0f;

    int iterations = 1;

    void CreateSpring(SoftBodyParticle* particleA, SoftBodyParticle* particleB);

    void CreateRandomSprings(int numOfSprings);

    void ApplyForce(const glm::vec3& force);
    void Update(float deltaTime);

    void UpdatePositions(float deltaTime);
    void UpdateNormals();

    void Draw(Shader* shader);

private:
    Mesh mesh;

    std::vector<SoftBodyParticle*> particles{};
    std::vector<SoftBodySpring*> springs{};

};

}  // namespace gdp1
