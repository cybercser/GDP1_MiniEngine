#include "renderer.h"

#include "model.h"

#include "scene.h"
#include "Render/skybox.h"
#include "Utils/camera.h"
#include "Core/game_object.h"
#include "Core/application.h"
#include "Physics/softbody.h"
#include "Core/timestep.h"
#include "Render/frustum.h"
#include "Resource/lod_system.h"
#include "Utils/timer.h"

#include <GLFW/glfw3.h>

using namespace glm;
using namespace std;

namespace gdp1 {

Renderer::Renderer() {
    this->viewFrustum = new Frustum();
    this->lodSystem = new LODSystem();
    this->projectionMatrix = glm::mat4(1.0f);
    this->viewMatrix = glm::mat4(1.0f);
}

void Renderer::Render(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, Timestep ts) {
    if (!scene || !camera) {
        LOG_ERROR("Scene or camera is null");
        return;
    }

    mat4 projection = camera->GetProjectionMatrix();
    mat4 view = camera->GetViewMatrix();
    mat4 umodel = mat4(1.0f);
    mat4 mv = view * umodel;
    mat3 normalMatrix = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

    glm::mat4 viewProjectionMatrix = projection * view;

    if (scene->HasFBO()) {
        scene->UseFBO();
        projection = glm::scale(projection, glm::vec3(1.0f, -1.0f, 1.0f));
    }

    ResetFrameBuffers();
    SetupShaders(scene, projection, view, umodel, normalMatrix);

    updateViewFrustum = isInstanced != setInstanced;

    if (viewFrustum->viewProjectionMatrix != viewProjectionMatrix || updateViewFrustum) {
        viewFrustum->Update(viewProjectionMatrix);
        culledObjects.clear();
        culledObjects = viewFrustum->GetCulledObjects(scene->GetGameObjectMap());
        updateViewFrustum = false;
    }

    isInstanced = setInstanced;

    if (setInstanced) {
        if (projectionMatrix != projection || viewMatrix != view)
            SetupInstancedRendering(projection, view, culledObjects);

        for (unordered_map<Model*, vector<glm::mat4>>::iterator it = instancesMap.begin(); it != instancesMap.end();
             it++) {
            scene->inst_shader_ptr_->Use();
            scene->inst_shader_ptr_->SetUniform("u_SetLit", false);
            scene->inst_shader_ptr_->SetUniform("u_UseLights", true);
            it->first->currentLODLevel = 0;
            it->first->Draw(scene->inst_shader_ptr_);
        }

        culledObjects = dynamicGoMap;
    }

    lodSystem->Update(camera, culledObjects);
    
    for (std::unordered_map<std::string, GameObject*>::iterator it = culledObjects.begin(); it != culledObjects.end();
         it++) {
        GameObject* go = it->second;
        if (go != nullptr && go->visible) {
            Model* model = go->model;
            if (model != nullptr) {
                // find shader
                unordered_map<string, Shader*>::const_iterator shaderIt = scene->m_ShaderMap.find(model->shaderName);
                if (shaderIt == scene->m_ShaderMap.end()) {
                    LOG_ERROR("Cannot find shader: " + model->shaderName);
                    continue;
                }

                Shader* shader = shaderIt->second;
                shader->Use();
                shader->SetUniform("u_Model", go->transform->WorldMatrix());
                shader->SetUniform("u_SetLit", go->setLit);
                shader->SetUniform("u_UseLights", true);

                if (!go->hasSoftBody) {
                    if (go->currentAnim != "") {
                        go->UpdateAnimation(shader, ts);

                        model->currentAnimation = go->currentAnimation;
                        model->prevAnimation = go->prevAnimation;
                    }
                    model->ResetInstancing();
                    model->Draw(shader);
                } else {
                    if (go->softBody) go->softBody->Draw(shader);
                }
            }
        }
    }

    // always draw the skybox at last
    if (renderSkybox) scene->skybox_ptr_->Draw(scene->skybox_shader_ptr_, view, projection);
    if (drawDebug) RenderDebug(scene, camera, ts);
}

void Renderer::RenderDebug(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, Timestep ts) {
    if (scene == nullptr || camera == nullptr) {
        LOG_ERROR("Scene or camera is null");
        return;
    }

    mat4 projection = camera->GetProjectionMatrix();
    mat4 view = camera->GetViewMatrix();
    mat4 model = mat4(1.0f);
    mat4 mv = view * model;
    mat3 normalMatrix = mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2]));

    {
        // update uniforms for debug shader
        scene->debug_shader_ptr_->Use();
        scene->debug_shader_ptr_->SetUniform("u_Model", model);
        scene->debug_shader_ptr_->SetUniform("u_View", view);
        scene->debug_shader_ptr_->SetUniform("u_Proj", projection);
    }

    std::unordered_map<std::string, GameObject*>& goMap = scene->m_GameObjectMap;
    std::unordered_map<std::string, Shader*>& shaderMap = scene->m_ShaderMap;

#pragma omp parallel for
    for (std::unordered_map<std::string, GameObject*>::iterator it = goMap.begin(); it != goMap.end(); it++) {
        GameObject* go = it->second;
        if (go != nullptr && go->visible) {
            Model* model = go->model;
            if (model != nullptr) {
                // find shader
                std::unordered_map<std::string, Shader*>::const_iterator shaderIt = shaderMap.find(model->shaderName);
                if (shaderIt == shaderMap.end()) {
                    LOG_ERROR("Cannot find shader: " + model->shaderName);
                    continue;
                }

                glm::mat4 worldMat = go->transform->WorldMatrix();

                Shader* shader = shaderIt->second;
                shader->Use();
                shader->SetUniform("u_Model", worldMat);
                shader->SetUniform("u_SetLit", go->setLit);
                shader->SetUniform("u_UseLights", true);

                model->DrawDebug(shader);
            }
        }
    }
}

void Renderer::SetupInstancedRendering(glm::mat4& projMatrix, glm::mat4& viewMatrix,
                                       std::unordered_map<std::string, GameObject*>& gameObjects) {
    this->projectionMatrix = projMatrix;
    this->viewMatrix = viewMatrix;

    instancesMap.clear();
    dynamicGoMap.clear();

#pragma omp parallel for
    for (std::unordered_map<std::string, GameObject*>::iterator it = gameObjects.begin(); it != gameObjects.end();
         it++) {
        GameObject* go = it->second;
        Model* model = go->model;

        if (model && go->isStatic && go->visible) {
            // Check if the model is already in instancesMap, and if not, add it
            // #pragma omp critical
            if (instancesMap.find(model) == instancesMap.end()) {
                instancesMap[model] = std::vector<glm::mat4>();
            }

            instancesMap[model].push_back(go->transform->WorldMatrix());
        }

        if (!go->isStatic) {
            // #pragma omp critical
            dynamicGoMap[go->name] = go;
        }
    }

#pragma omp parallel for
    for (std::unordered_map<Model*, std::vector<glm::mat4>>::iterator it = instancesMap.begin();
         it != instancesMap.end(); it++) {
        it->first->SetupInstancing(it->second, true);
    }

    return;
}

void Renderer::SetInstanced(bool setInstanced) {
    this->setInstanced = setInstanced;
    this->updateViewFrustum = true;
}

void Renderer::SetupShaders(std::shared_ptr<Scene> scene, glm::mat4 projection, glm::mat4 view, glm::mat4 model,
                            glm::mat3 normalMatrix) {
    // get the directional light
    DirectionalLight* dirLight = scene->FindDirectionalLightByName("Sun");

    // update uniforms for default shader
    scene->lit_shader_ptr_->Use();
    scene->lit_shader_ptr_->SetUniform("u_Model", model);
    scene->lit_shader_ptr_->SetUniform("u_View", view);
    scene->lit_shader_ptr_->SetUniform("u_Proj", projection);
    scene->lit_shader_ptr_->SetUniform("u_NormalMat", normalMatrix);

    vec3 lightDirViewSpace = normalMatrix * dirLight->direction;

    // directional light
    scene->lit_shader_ptr_->SetUniform("u_DirLight.dir", lightDirViewSpace);
    scene->lit_shader_ptr_->SetUniform("u_NumPointLights", (int)scene->m_PointLightMap.size());
    scene->lit_shader_ptr_->SetUniform("u_UseLights", true);

    int lightIndex = 0;
    for (std::unordered_map<std::string, PointLight*>::iterator it = scene->m_PointLightMap.begin();
         it != scene->m_PointLightMap.end(); it++, lightIndex++) {
        PointLight* pointLight = it->second;
        vec4 lightPosInViewSpace = glm::vec4(view * glm::vec4(pointLight->position, 1.0f));
        scene->lit_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].color",
                                           pointLight->color);
        scene->lit_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].intensity",
                                           pointLight->intensity);
        scene->lit_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].c", pointLight->constant);
        scene->lit_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].l", pointLight->linear);
        scene->lit_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].q",
                                           pointLight->quadratic);
        scene->lit_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].pos",
                                           glm::vec3(lightPosInViewSpace));
    }

    // update uniforms for instance shader same as default
    scene->inst_shader_ptr_->Use();
    scene->inst_shader_ptr_->SetUniform("u_View", view);
    scene->inst_shader_ptr_->SetUniform("u_Proj", projection);
    scene->inst_shader_ptr_->SetUniform("u_NormalMat", normalMatrix);

    scene->inst_shader_ptr_->SetUniform("u_DirLight.dir", lightDirViewSpace);
    scene->inst_shader_ptr_->SetUniform("u_NumPointLights", (int)scene->m_PointLightMap.size());
    scene->inst_shader_ptr_->SetUniform("u_UseLights", true);

    lightIndex = 0;
    for (std::unordered_map<std::string, PointLight*>::iterator it = scene->m_PointLightMap.begin();
         it != scene->m_PointLightMap.end(); it++, lightIndex++) {
        PointLight* pointLight = it->second;
        vec4 lightPosInViewSpace = glm::vec4(view * glm::vec4(pointLight->position, 1.0f));
        scene->inst_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].color",
                                            pointLight->color);
        scene->inst_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].intensity",
                                            pointLight->intensity);
        scene->inst_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].c",
                                            pointLight->constant);
        scene->inst_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].l", pointLight->linear);
        scene->inst_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].q",
                                            pointLight->quadratic);
        scene->inst_shader_ptr_->SetUniform("u_PointLights[" + std::to_string(lightIndex) + "].pos",
                                            glm::vec3(lightPosInViewSpace));
    }

    // update uniforms for untextured shader
    scene->untextured_shader_ptr_->Use();
    scene->untextured_shader_ptr_->SetUniform("u_Model", model);
    scene->untextured_shader_ptr_->SetUniform("u_View", view);
    scene->untextured_shader_ptr_->SetUniform("u_Proj", projection);
    scene->untextured_shader_ptr_->SetUniform("u_NormalMat", normalMatrix);
    scene->untextured_shader_ptr_->SetUniform("u_DirLight.dir", lightDirViewSpace);

    // update uniforms for unlit shader
    scene->unlit_shader_ptr_->Use();
    scene->unlit_shader_ptr_->SetUniform("u_Model", model);
    scene->unlit_shader_ptr_->SetUniform("u_View", view);
    scene->unlit_shader_ptr_->SetUniform("u_Proj", projection);

    isShadersInitialized = true;
}

void Renderer::ResetFrameBuffers() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Renderer::IsObjectVisible(glm::mat4& projMatrix, glm::mat4& viewMatrix, GameObject* object) {
    Bounds objBounds = object->model->bounds;
    objBounds.TransformBounds(object->transform->WorldMatrix());

    glm::vec4 minClip = projMatrix * viewMatrix * glm::vec4(objBounds.GetMin(), 1.0);
    glm::vec4 maxClip = projMatrix * viewMatrix * glm::vec4(objBounds.GetMax(), 1.0);

    // Check if the AABB is completely outside any of the six planes of the frustum
    if (maxClip.x < -minClip.w || minClip.x > minClip.w || maxClip.y < -minClip.w || minClip.y > minClip.w ||
        maxClip.z < -minClip.w || minClip.z > minClip.w) {
        return false;  // Object is completely outside the frustum
    }

    return true;  // Object is outside the view frustum
}

std::vector<GameObject*> Renderer::PerformFrustumCulling(glm::mat4& projMatrix, glm::mat4& viewMatrix,
                                                         std::unordered_map<std::string, GameObject*>& gameObjects) {
    this->projectionMatrix = projMatrix;
    this->viewMatrix = viewMatrix;

    std::vector<GameObject*> visibleObjects;

    for (std::unordered_map<std::string, GameObject*>::iterator it = gameObjects.begin(); it != gameObjects.end();
         it++) {
        if (IsObjectVisible(projMatrix, viewMatrix, it->second)) {
            visibleObjects.push_back(it->second);
        }
    }

    return visibleObjects;
}

}  // namespace gdp1
