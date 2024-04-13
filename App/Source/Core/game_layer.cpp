#include "game_layer.h"

using namespace gdp1;
using namespace gdp1::utils;

#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"

#include "Utils/softbody_utils.h"

GameLayer::GameLayer()
    : Layer("Game") {}

void GameLayer::OnAttach() {
    EnableGLDebugging();
    SetGLDebugLogLevel(DebugLogLevel::High);

    // Create Scene
    m_Scene = std::make_shared<Scene>("Assets/Levels/fps_test.json");

    // Initialize things here

    zombie1 = m_Scene->FindObjectByName("zombie1");
    zombie2 = m_Scene->FindObjectByName("zombie");

    zombie1->SetCurrentAnimation("zombie_idle");
    zombie2->SetCurrentAnimation("zombie_idle");

    // Add Player and objects to the scene
    AddPlayer();

    // Initialize Audio Manager
    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->Initialize();

    // Play SFX Music initially - later change to handling by lua scripts
    gdp1::AudioSourceDesc sfxAudio = m_Scene->GetLevelDesc().audioSourceDescs[0];
    m_audioManager.get()->LoadAudio(sfxAudio.filepath.c_str());
    m_audioManager.get()->PlayAudio(sfxAudio.filepath.c_str(), CHANNELGROUP_SFX_INDEX);

    // Initialize Database
    db = std::make_shared<SQLiteDatabase>("Assets/Data/game_data.db");
    db->open();

    // init the camera
    const CameraDesc& camDesc = m_Scene->GetLevelDesc().cameraDescs[0];
    m_Player->SetFPSCamera(camDesc);

    // init the renderer
    m_Renderer = std::make_unique<Renderer>();

    // init physics engine
    m_Physics = std::make_unique<Physics>(m_Scene.get(), m_Scene->GetLevelDesc());
    m_Physics->StartSoftBodyThreads();

    // configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GameLayer::OnDetach() {}

void GameLayer::OnEvent(gdp1::Event& event) {
    // m_FlyCamera->OnEvent(event);
    m_Player->OnEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) { return false; });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) { return false; });
    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) { return false; });
}

void GameLayer::OnUpdate(gdp1::Timestep ts) {

    // m_FlyCamera->OnUpdate(ts);
    m_Player->Update(ts);

    m_Physics->FixedUpdate(ts);
    m_Scene->Update(ts);

    m_Renderer->Render(m_Scene, m_Player->fps_camera_ptr_.get()->GetCamera(), ts);
}

void GameLayer::OnImGuiRender() {
    ImGui::Begin("Controls");
    ImGui::Text("WASD to move, mouse to look around");
    // add label to show the camera parameters
    const glm::vec3& pos = m_Player->fps_camera_ptr_.get()->GetPosition();
    const glm::vec3& up = m_Player->fps_camera_ptr_.get()->GetUp();
    float yaw = m_Player->fps_camera_ptr_.get()->GetYaw();
    float pitch = m_Player->fps_camera_ptr_.get()->GetPitch();
    float fov = m_Player->fps_camera_ptr_.get()->GetFov();

    ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)\nUp: (%.1f, %.1f, %.1f)\nYaw: %.1f, Pitch: %.1f, FOV: %.1f", pos.x,
                pos.y, pos.z, up.x, up.y, up.z, yaw, pitch, fov);

    ImGui::Text("Player Position: (%.3f, %.3f, %.3f)", m_Player->transform->localPosition.x,
                m_Player->transform->localPosition.y, m_Player->transform->localPosition.z);

    ImGui::Checkbox("Enable Skybox", &m_Renderer->renderSkybox);
    ImGui::Checkbox("Draw Debug", &m_Renderer->drawDebug);
    ImGui::Checkbox("Set Instanced", &m_Renderer->setInstanced);

    ImGui::End();

    ImGui::Begin("Change Zombie 1 Animations");
    static int selectedAnimation = 0;
    const char* animationNames[] = {"Idle", "Walk", "Run", "Attack", "Scream", "Bullet Hit Reaction", "Crawl", "Death"};
    for (int i = 0; i < IM_ARRAYSIZE(animationNames); i++) {
        if (ImGui::RadioButton(animationNames[i], &selectedAnimation, i)) {
            // Call a function with a parameter based on the selected radio button
            switch (selectedAnimation) {
                case 0:
                    zombie1->SetCurrentAnimation("zombie_idle");
                    break;
                case 1:
                    zombie1->SetCurrentAnimation("zombie_walking");
                    break;
                case 2:
                    zombie1->SetCurrentAnimation("zombie_running");
                    break;
                case 3:
                    zombie1->SetCurrentAnimation("zombie_attack");
                    break;
                case 4:
                    zombie1->SetCurrentAnimation("zombie_scream");
                    break;
                case 5:
                    zombie1->SetCurrentAnimation("zombie_reaction_hit");
                    break;
                case 6:
                    zombie1->SetCurrentAnimation("zombie_crawl");
                    break;
                case 7:
                    zombie1->SetCurrentAnimation("zombie_death");
                    break;
                default:
                    break;
            }
        }
    }

    ImGui::End();

    ImGui::Begin("Change Zombie 2 Animations");

    static int selectedAnimation1 = 0;
    for (int i = 0; i < IM_ARRAYSIZE(animationNames); i++) {
        if (ImGui::RadioButton(animationNames[i], &selectedAnimation1, i)) {
            // Call a function with a parameter based on the selected radio button
            switch (selectedAnimation1) {
                case 0:
                    zombie2->SetCurrentAnimation("zombie_idle");
                    break;
                case 1:
                    zombie2->SetCurrentAnimation("zombie_walking");
                    break;
                case 2:
                    zombie2->SetCurrentAnimation("zombie_running");
                    break;
                case 3:
                    zombie2->SetCurrentAnimation("zombie_attack");
                    break;
                case 4:
                    zombie2->SetCurrentAnimation("zombie_scream");
                    break;
                case 5:
                    zombie2->SetCurrentAnimation("zombie_reaction_hit");
                    break;
                case 6:
                    zombie2->SetCurrentAnimation("zombie_crawl");
                    break;
                case 7:
                    zombie2->SetCurrentAnimation("zombie_death");
                    break;
                default:
                    break;
            }
        }
    }

    ImGui::End();
}

float RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

void GameLayer::CreateSpheres(gdp1::Scene* scene, int numRaindrops) {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < numRaindrops; ++i) {
        // Generate random position, size, and other properties
        float x = RandomFloat(-7.0f, 7.0f);      // Random X position
        float y = RandomFloat(0.0f, 4.0f);       // Random Y position (height)
        float z = RandomFloat(-7.0f, 7.0f);      // Random Z position
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
        desc.isStatic = true;

        // Create the raindrop GameObject and add it to the scene
        GameObject* raindrop = new GameObject(m_Scene.get(), desc);

        RigidbodyDesc rigidbodyDesc;
        rigidbodyDesc.active = true;
        rigidbodyDesc.collider = "SPHERE";
        rigidbodyDesc.invMass = 1.0f;
        rigidbodyDesc.objectName = desc.name;
        rigidbodyDesc.orientation = glm::vec3(0.0f);
        rigidbodyDesc.position = glm::vec3(0.0f);
        rigidbodyDesc.velocity = glm::vec3(0.0f);

        scene->AddGameObject(raindrop);  // Add the raindrop to the scene (assuming this function exists)
    }
}

void GameLayer::AddPlayer() {
    GameObjectDesc playerDesc;

    playerDesc.name = "Player";
    playerDesc.modelName = "PlayerHands";
    playerDesc.visible = true;
    playerDesc.transform.localPosition = glm::vec3(2.5f, 0.3f, -4.5f);
    playerDesc.transform.localScale = glm::vec3(0.5f);
    playerDesc.transform.localEulerAngles = glm::vec3(0.0f, 30.f, 0.f);
    playerDesc.setLit = true;
    playerDesc.parentName = "";
    playerDesc.isStatic = false;

    RigidbodyDesc rigidBodyDesc;

    rigidBodyDesc.active = true;
    rigidBodyDesc.applyGravity = false;
    rigidBodyDesc.collider = "MESH";
    rigidBodyDesc.invMass = 1.0f;
    rigidBodyDesc.objectName = "Player";
    rigidBodyDesc.position = playerDesc.transform.localPosition;
    rigidBodyDesc.velocity = glm::vec3(0.f);
    rigidBodyDesc.applyGravity = false;
    rigidBodyDesc.orientation = playerDesc.transform.localEulerAngles;

    m_Scene->GetLevelDesc().rigidbodyDescs.push_back(rigidBodyDesc);

    m_Player = new Player(m_Scene.get(), playerDesc);
    m_Player->model = m_Scene->FindModelByName(playerDesc.modelName);
    m_Player->SetCurrentAnimation("Player_Idle");

    m_Scene->AddGameObject(m_Player);
}

void GameLayer::AddCoins() {
    for (int i = 0; i < 30; i++) {
        // Generate random position, size, and other properties
        float x = RandomFloat(-20.0f, 20.0f);  // Random X position
        float z = RandomFloat(-20.0f, 20.0f);  // Random Z position

        GameObjectDesc coinDesc;

        coinDesc.name = "Coin" + std::to_string(i);
        coinDesc.modelName = "Coin";
        coinDesc.visible = true;
        coinDesc.transform.localPosition = glm::vec3(x, 0.7f, z);
        coinDesc.transform.localScale = glm::vec3(0.005f);
        coinDesc.transform.localEulerAngles = glm::vec3(0.0f);
        coinDesc.setLit = true;
        coinDesc.parentName = "";

        RigidbodyDesc rigidBodyDesc;

        rigidBodyDesc.active = true;
        rigidBodyDesc.collider = "SPHERE";
        rigidBodyDesc.invMass = 1.0f;
        rigidBodyDesc.applyGravity = false;
        rigidBodyDesc.objectName = "Coin" + std::to_string(i);
        rigidBodyDesc.position = coinDesc.transform.localPosition;
        rigidBodyDesc.velocity = glm::vec3(0.f);
        rigidBodyDesc.orientation = glm::vec3(0.f);

        m_Scene->GetLevelDesc().rigidbodyDescs.push_back(rigidBodyDesc);
        m_Scene->AddGameObject(new Collectible(m_Scene.get(), coinDesc, CollectibleType::COIN));
    }
}
