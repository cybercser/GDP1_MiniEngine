#pragma once

#include <common.h>

namespace gdp1 {

class Scene;
class GameObject;
class Model;

class ParticleSystem {
public:
    ParticleSystem(std::shared_ptr<Scene> scene, unsigned int numParticles);
    ~ParticleSystem();

    void Render();

private:
    unsigned int numParticles;

    std::shared_ptr<Scene> scene;

    std::vector<GameObject*> particles;
    std::vector<glm::mat4> instanceMatrices;

    Model* particleModel;
};

}  // namespace gdp1