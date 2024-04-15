#pragma once

#include <common.h>

namespace gdp1 {

// Forward declarations
class Scene;
class Camera;
class Timestep;
class Model;
class GameObject;
class Frustum;
class LODSystem;

class Renderer {
public:
    Renderer();
    void Render(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, Timestep ts);
    void RenderDebug(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, Timestep ts);
    void SetupInstancedRendering(glm::mat4& projMatrix, glm::mat4& viewMatrix,
                                 std::unordered_map<std::string, GameObject*>& gameObjects);
    void SetInstanced(bool setInstanced);

    bool updateViewFrustum = true;
    bool setInstanced = false;
    bool renderSkybox = true;
    bool drawDebug = false;

    bool isInstanced = true;

private:
    bool isShadersInitialized = false;
    float blendFactor = 0.0f;

    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    std::unordered_map<Model*, std::vector<glm::mat4>> instancesMap;
    std::unordered_map<std::string, GameObject*> dynamicGoMap;
    std::vector<GameObject*> fcGoMap;

    Frustum* viewFrustum;
    LODSystem* lodSystem;

    std::unordered_map<std::string, GameObject*> culledObjects;

private:
    void SetupShaders(std::shared_ptr<Scene> scene, glm::mat4 projection, glm::mat4 view, glm::mat4 model,
                      glm::mat3 normalMat);
    void ResetFrameBuffers();
    bool IsObjectVisible(glm::mat4& projMatrix, glm::mat4& viewMatrix, GameObject* object);
    std::vector<GameObject*> PerformFrustumCulling(glm::mat4& projMatrix, glm::mat4& viewMatrix,
                                                   std::unordered_map<std::string, GameObject*>& gameObjects);
};

}  // namespace gdp1
