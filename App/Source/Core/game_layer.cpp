#include "game_layer.h"

using namespace gdp1;
using namespace gdp1::utils;

#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"

#include "Core/game_object.h"
#include "Utils/softbody_utils.h"

GameLayer::GameLayer()
    : Layer("Game") {}

void GameLayer::OnAttach() {
    EnableGLDebugging();

    m_Scene = std::make_shared<Scene>("Assets/Levels/fps_test.json");
    fbo_Scene = std::make_shared<Scene>("Assets/Levels/fps_test_1.json");
    fbo_Scene->CreateFBO();

    // init the camera
    const CameraDesc& camDesc = m_Scene->GetLevelDesc().cameraDescs[0];

    m_FlyCamera = std::make_shared<FlyCameraController>(camDesc, 16.0f / 9.0f, 10.0f, 2.0f);
    m_FboCamera = std::make_shared<FlyCameraController>(camDesc, 16.0f / 9.0f, 10.0f, 2.0f);

    // init the renderer
    m_Renderer = std::make_unique<Renderer>();

    // init physics engine
    m_Physics = std::make_unique<Physics>(m_Scene.get(), m_Scene->GetLevelDesc());
    m_Physics->StartSoftBodyThreads();

    // configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GameLayer::OnDetach() {}

void GameLayer::OnEvent(gdp1::Event& event) {
    m_FlyCamera->OnEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) { return false; });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) { return false; });
    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) { return false; });
}

void GameLayer::OnUpdate(gdp1::Timestep ts) {
    m_FlyCamera->OnUpdate(ts);

    m_Physics->FixedUpdate(ts);
    m_Scene->Update(ts);
    m_Renderer->Render(fbo_Scene, m_FboCamera->GetCamera());
    m_Renderer->Render(m_Scene, m_FlyCamera->GetCamera(), fbo_Scene->GetFBO()->colorTextureId);
}

void GameLayer::OnImGuiRender() {
    ImGui::Begin("Controls");
    ImGui::Text("WASD to move, mouse to look around");
    // add label to show the camera parameters
    const glm::vec3& pos = m_FlyCamera->GetPosition();
    const glm::vec3& up = m_FlyCamera->GetUp();
    float yaw = m_FlyCamera->GetYaw();
    float pitch = m_FlyCamera->GetPitch();
    float fov = m_FlyCamera->GetFov();

    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)\nUp: (%.1f, %.1f, %.1f)\nYaw: %.1f, Pitch: %.1f, FOV: %.1f", pos.x,
                pos.y, pos.z, up.x, up.y, up.z, yaw, pitch, fov);
    ImGui::End();
}
