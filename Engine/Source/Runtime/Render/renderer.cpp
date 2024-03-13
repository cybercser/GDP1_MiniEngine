#include "renderer.h"

#include "model.h"

#include "scene.h"
#include "Render/skybox.h"
#include "Utils/camera.h"
#include "Core/game_object.h"
#include "Physics/softbody.h"

using namespace glm;

namespace gdp1 {

void Renderer::Render(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) {
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

        //// update uniforms for debug shader
        // scene->debug_shader_ptr_->Use();
        // scene->debug_shader_ptr_->SetUniform("u_Model", model);
        // scene->debug_shader_ptr_->SetUniform("u_View", view);
        // scene->debug_shader_ptr_->SetUniform("u_Proj", projection);

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

                Shader* shader = shaderIt->second;
                shader->Use();
                shader->SetUniform("u_Model", go->transform->WorldMatrix());

                if (!go->hasSoftBody)
                    model->Draw(shader);
                else {
                    if (go->softBody) go->softBody->Draw(shader);
                }
            }
        }
    }

    // always draw the skybox at last
    scene->skybox_ptr_->Draw(scene->skybox_shader_ptr_, view, projection);
}

}  // namespace gdp1
