#pragma once

#include <common.h>

namespace gdp1 {

// Forward declarations
class Scene;
class Camera;
class Timestep;
class Model;
class GameObject;

class Renderer {
public:
    Renderer() = default;
    void Render(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, Timestep ts, bool renderSkybox = true,
                bool setInstanced = false);
    void RenderDebug(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, Timestep ts,
                     bool renderSkybox = true);
    void SetupInstancedRendering(std::shared_ptr<Scene> scene);
    void SetInstanced(bool setInstanced);

private:
    //bool isInstanced = false;
    bool initializedInstancing = false;
    bool isShadersInitialized = false;
    float blendFactor = 0.0f;
    std::unordered_map<Model*, std::vector<glm::mat4>> instancesMap;
    std::unordered_map<std::string, GameObject*> dynamicGoMap;

private:
    void SetupShaders(std::shared_ptr<Scene> scene, glm::mat4 projection, glm::mat4 view, glm::mat4 model,
                      glm::mat3 normalMat);
    void ResetFrameBuffers();
};

}  // namespace gdp1
