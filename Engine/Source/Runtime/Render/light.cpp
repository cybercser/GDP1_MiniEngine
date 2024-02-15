#include "light.h"

#include "Resource/texture.h"
#include "shader.h"

namespace gdp1 {

unsigned int SpotLight::InitCookie() {
    cookieTexId = Texture::LoadTexture(cookie);
    return cookieTexId;
}

void SpotLight::EnableCookie(std::shared_ptr<Shader> shader, const glm::vec3& projUp) {
    glm::vec3 projPos = position;
    glm::vec3 projAt = position + glm::normalize(direction);
    glm::mat4 projView = glm::lookAt(projPos, projAt, projUp);
    glm::mat4 projProj = glm::perspective(glm::radians(30.0f), 1.0f, 0.2f, 1000.0f);
    glm::mat4 bias = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f));
    bias = glm::scale(bias, glm::vec3(0.5f));
    shader->Use();
    shader->SetUniform("u_ProjectorMat", bias * projProj * projView);
    shader->SetUniform("u_UseProjTex", true);
    // shader->setUniform("u_ProjectorTex", 4);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, cookieTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void SpotLight::DisableCookie(std::shared_ptr<Shader> shader) {
    shader->Use();
    shader->SetUniform("u_UseProjTex", false);
}

}  // namespace gdp1
