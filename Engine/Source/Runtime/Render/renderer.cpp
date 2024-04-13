#include "renderer.h"

#include "model.h"

#include "scene.h"
#include "Render/skybox.h"
#include "Utils/camera.h"
#include "Core/game_object.h"
#include "Core/application.h"
#include "Physics/softbody.h"
#include "Core/timestep.h"

#include <GLFW/glfw3.h>

using namespace glm;
using namespace std;

namespace gdp1 {

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

    if (scene->HasFBO()) {
        scene->UseFBO();
        projection = glm::scale(projection, glm::vec3(1.0f, -1.0f, 1.0f));
    }

    ResetFrameBuffers();
    SetupShaders(scene, projection, view, umodel, normalMatrix);

    unordered_map<string, Model*>& modelMap = scene->m_ModelMap;
    std::unordered_map<std::string, GameObject*> goMap;

    if (setInstanced) {
        if (projectionMatrix != projection || viewMatrix != view) SetupInstancedRendering(projection, view, scene);

        for (unordered_map<Model*, vector<glm::mat4>>::iterator it = instancesMap.begin(); it != instancesMap.end();
             it++) {
            scene->inst_shader_ptr_->Use();
            it->first->Draw(scene->inst_shader_ptr_);
        }

        goMap = dynamicGoMap;

        /*if (projectionMatrix != projection || viewMatrix != view)
            fcGoMap = PerformFrustumCulling(projection, view, goMap);*/
    } else {
        initializedInstancing = false;

        for (std::unordered_map<Model*, std::vector<glm::mat4>>::iterator it = instancesMap.begin();
             it != instancesMap.end(); it++) {
            it->first->ResetInstancing();
        }

        goMap = scene->m_GameObjectMap;

        /*if (projectionMatrix != projection || viewMatrix != view)
            fcGoMap = PerformFrustumCulling(projection, view, goMap);*/
    }

    for (std::unordered_map<std::string, GameObject*>::iterator it = goMap.begin(); it != goMap.end(); it++) {
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

void Renderer::SetupInstancedRendering(glm::mat4& projMatrix, glm::mat4& viewMatrix, std::shared_ptr<Scene> scene) {
    instancesMap.clear();
    dynamicGoMap.clear();

    std::unordered_map<std::string, GameObject*>& goMap = scene->m_GameObjectMap;
    std::unordered_map<std::string, Shader*>& shaderMap = scene->m_ShaderMap;

    std::vector<GameObject*> tempMap = PerformFrustumCulling(projMatrix, viewMatrix, goMap);

    for (const auto& go : tempMap) {
        Model* model = go->model;

        if (model && go->isStatic && go->visible) {
            // Check if the model is already in instancesMap, and if not, add it
            if (instancesMap.find(model) == instancesMap.end()) {
                instancesMap[model] = std::vector<glm::mat4>();
            }

            instancesMap[model].push_back(go->transform->WorldMatrix());
        }

        if (!go->isStatic) {
            dynamicGoMap[go->name] = go;
        }
    }

    for (std::unordered_map<Model*, std::vector<glm::mat4>>::iterator it = instancesMap.begin();
         it != instancesMap.end(); it++) {
        Model* model = it->first;
        if (model != nullptr) {
            model->ResetInstancing();
            model->SetupInstancing(it->second);
        }
    }

    initializedInstancing = true;

    return;
}

void Renderer::SetInstanced(bool setInstanced) { this->setInstanced = setInstanced; }

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
