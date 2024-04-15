#pragma once

#include <common.h>

namespace gdp1 {

class Scene;
class GameObject;
class Model;
class Camera;
class Bounds;

class ParticleSystem {
public:
    ParticleSystem(std::shared_ptr<Scene> scene, unsigned int numParticles);
    ~ParticleSystem();

    void Render(std::shared_ptr<Camera> camera);

private:
    unsigned int numParticles;

    std::shared_ptr<Scene> scene;

    std::unordered_map<std::string, GameObject*> particles;
    std::vector<glm::mat4> instanceMatrices;

    Bounds* bounds;
    Model* particleModel;
};

}  // namespace gdp1