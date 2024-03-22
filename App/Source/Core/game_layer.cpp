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

    //m_Scene = std::make_shared<Scene>("Assets/Levels/fps_test_1.json");
    m_Scene = std::make_shared<Scene>("Assets/Levels/scene_monitors.json");

    fbo_Scene = std::make_shared<Scene>("Assets/Levels/fps_test.json");
    fbo_Scene->CreateFBO();

    fbo_Scene_1 = std::make_shared<Scene>("Assets/Levels/fps_test.json");
    fbo_Scene_1->CreateFBO();

    // init the camera
    const CameraDesc& camDesc = m_Scene->GetLevelDesc().cameraDescs[0];
    const CameraDesc& fbo_CamDesc = fbo_Scene->GetLevelDesc().cameraDescs[0];
    CameraDesc fbo_CamDesc_1 = fbo_Scene_1->GetLevelDesc().cameraDescs[0];
    fbo_CamDesc_1.position = glm::vec3(9.6f, 11.2f, -0.8f);
    fbo_CamDesc_1.up = glm::vec3(-0.5f, 0.8f, -0.2f);
    fbo_CamDesc_1.yaw = 560.7f;
    fbo_CamDesc_1.pitch = -34.8f;

    m_FlyCamera = std::make_unique<FlyCameraController>(camDesc, 16.0f / 9.0f, 10.0f, 2.0f);
    m_FboCamera = std::make_unique<FlyCameraController>(fbo_CamDesc, 1.0f, 10.0f, 2.0f);
    m_FboCamera_1 = std::make_unique<FlyCameraController>(fbo_CamDesc_1, 1.0f, 10.0f, 2.0f);

    // init the renderer
    m_Renderer = std::make_unique<Renderer>();

    CreateRaindropObjects(fbo_Scene.get(), 50);
    CreateRaindropObjects(fbo_Scene_1.get(), 50);

    gdp1::GameObject* gameObject = m_Scene.get()->FindObjectByName("RetroTVScreen_1");
    gameObject->UseChromaticAberration = true;
    gameObject->UseNightVision = false;
    gameObject->fboTextureId = fbo_Scene->GetFBO()->colorTextureId;

    gdp1::GameObject* gameObject1 = m_Scene.get()->FindObjectByName("RetroTVScreen_2");
    gameObject1->UseChromaticAberration = false;
    gameObject1->UseNightVision = true;
    gameObject1->fboTextureId = fbo_Scene_1->GetFBO()->colorTextureId;

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
    if (Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT)) {
        m_FboCamera_1->OnEvent(event);
    } else {
        m_FlyCamera->OnEvent(event);
    }

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) { return false; });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) { return false; });
    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) { return false; });
}

void GameLayer::OnUpdate(gdp1::Timestep ts) {
    if (Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT)) {
        m_FboCamera_1->OnUpdate(ts);
    } else {
        m_FlyCamera->OnUpdate(ts);
    }

    m_Physics->FixedUpdate(ts);
    m_Scene->Update(ts);

    m_Renderer->Render(fbo_Scene, m_FboCamera->GetCamera(), true);
    m_Renderer->Render(fbo_Scene_1, m_FboCamera_1->GetCamera(), true);
    m_Renderer->Render(m_Scene, m_FlyCamera->GetCamera(), enableSkyBox);
}

void GameLayer::OnImGuiRender() {
    ImGui::Begin("Controls");
    ImGui::Text("WASD to move, mouse to look around");
    // add label to show the camera parameters
    const glm::vec3& pos = m_FboCamera_1->GetPosition();
    const glm::vec3& up = m_FboCamera_1->GetUp();
    float yaw = m_FboCamera_1->GetYaw();
    float pitch = m_FboCamera_1->GetPitch();
    float fov = m_FboCamera_1->GetFov();

    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)\nUp: (%.1f, %.1f, %.1f)\nYaw: %.1f, Pitch: %.1f, FOV: %.1f", pos.x,
                pos.y, pos.z, up.x, up.y, up.z, yaw, pitch, fov);
    ImGui::Checkbox("Enable Skyobx", &enableSkyBox);
    ImGui::End();
}

float RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

void GameLayer::CreateRaindropObjects(gdp1::Scene* scene, int numRaindrops) {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < numRaindrops; ++i) {
        // Generate random position, size, and other properties
        float x = RandomFloat(-7.0f, 7.0f);    // Random X position
        float y = RandomFloat(0.0f, 7.0f);      // Random Y position (height)
        float z = RandomFloat(-7.0f, 7.0f);    // Random Z position
        float scale = RandomFloat(0.05f, 0.2f);  // Random scale
        bool isVisible = true;                   // Raindrop is initially visible

        glm::vec4 randomColor(RandomFloat(0.0f, 1.0f), RandomFloat(0.0f, 1.0f), RandomFloat(0.0f, 1.0f), 1.0f);

        // Create a GameObjectDesc for the raindrop
        GameObjectDesc desc;
        desc.name = "Raindrop" + std::to_string(i);
        desc.modelName = "sphere";  // Assuming "SphereModel" is the name of your sphere model
        desc.visible = isVisible;
        desc.hasFBO = false;  // Assuming raindrops don't have an FBO
        desc.transform.localPosition = {x, y, z};
        desc.transform.localScale = {scale, scale, scale};
        desc.setLit = true;

        // Create the raindrop GameObject and add it to the scene
        GameObject* raindrop = new GameObject(m_Scene.get(), desc);

        RigidbodyDesc rigidbodyDesc;
        rigidbodyDesc.active = true;
        rigidbodyDesc.collider = "SPHERE";
        rigidbodyDesc.invMass = 1.0f;
        rigidbodyDesc.objectName = desc.name;
        rigidbodyDesc.orientation = glm::vec3(0.0f);
        rigidbodyDesc.position = glm::vec3(0.0f);
        rigidbodyDesc.velocity =
            glm::vec3(RandomFloat(-0.1f, 0.1f), -RandomFloat(0.01f, 0.05f), RandomFloat(-0.1f, 0.1f));

        PointLight pointLight;
        pointLight.color = randomColor;
        pointLight.name = desc.name;
        pointLight.position = {x, y, z};
        pointLight.intensity = 0.7f;
        pointLight.constant = 0.5f;
        pointLight.linear = 2.0f;
        pointLight.quadratic = 1.0f;

        scene->AddPointLight(pointLight);

        // m_Scene->GetLevelDesc().rigidbodyDescs.push_back(rigidbodyDesc);

        scene->AddGameObject(raindrop);  // Add the raindrop to the scene (assuming this function exists)
    }
}
