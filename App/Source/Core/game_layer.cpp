#include "game_layer.h"

using namespace gdp1;
using namespace gdp1::utils;

GameLayer::GameLayer()
    : Layer("Game") {}

void GameLayer::OnAttach() {
    EnableGLDebugging();

    // init the scene
    LevelLoader loader;
    std::string levelFile = "Assets/Levels/fps_test.json";
    if (loader.LoadLevel(levelFile)) {
        LOG_INFO("Load scene successfully");
    } else {
        LOG_ERROR("Failed to load scene: {}", levelFile);
        return;
    }
    const LevelDesc& levelJson = loader.GetLevelDesc();
    m_Scene = std::make_shared<Scene>(levelJson);

    // init the camera
    const CameraDesc& camDesc = levelJson.cameraDescs[0];
    m_FlyCamera =
        std::make_shared<FlyCameraController>(camDesc.position, camDesc.up, camDesc.yaw, camDesc.pitch, camDesc.fov,
                                              16.0f / 9.0f, camDesc.nearZ, camDesc.farZ, 10.0f, 2.0f);

    // init the renderer
    m_Renderer = std::make_unique<Renderer>();

    // configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GameLayer::OnDetach() {}

void GameLayer::OnEvent(gdp1::Event& event) {
    LOG_TRACE("{0}", event);

    m_FlyCamera->OnEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) { return false; });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) { return false; });
    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) { return false; });
}

void GameLayer::OnUpdate(gdp1::Timestep ts) {
    // LOG_INFO("GameLayer::OnUpdate");

    m_FlyCamera->OnUpdate(ts);

    m_Scene->Update(ts);
    glClearColor(0.1f, 1.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_Renderer->Render(m_Scene, m_FlyCamera->GetCamera());
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
