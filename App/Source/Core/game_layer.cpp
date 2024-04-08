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
    zombie1->SetCurrentAnimation("zombie_crawl");

    zombie2 = m_Scene->FindObjectByName("zombie");
    zombie2->SetCurrentAnimation("zombie_walking");

    // Add Player and objects to the scene
    AddPlayer();

    // m_ParticleSystem = std::make_unique<ParticleSystem>(m_Scene, 1);

    //AddCoins();
    CreateSpheres(m_Scene.get(), 1000);

    // Initialize Audio Manager
    m_audioManager = std::make_unique<AudioManager>();
    m_audioManager->Initialize();

    // Play SFX Music initially - later change to handling by lua scripts
    // gdp1::AudioSourceDesc sfxAudio = m_Scene->GetLevelDesc().audioSourceDescs[0];
    // m_audioManager.get()->LoadAudio(sfxAudio.filepath.c_str());
    // m_audioManager.get()->PlayAudio(sfxAudio.filepath.c_str(), CHANNELGROUP_SFX_INDEX);

    // Initialize Database
    db = std::make_shared<SQLiteDatabase>("Assets/Data/game_data.db");
    db->open();

    // Check sample usage of database operations
    gameplayManager = new GameplayManager(*db.get());

    gameplayManager->playerKilledZombie(m_Player->id, 1);
    gameplayManager->playerKilledZombie(m_Player->id, 2);

    int kills = gameplayManager->getPlayerKillCount(m_Player->id);
    LOG_ERROR("Player killed: {}", kills);

    // init the camera
    const CameraDesc& camDesc = m_Scene->GetLevelDesc().cameraDescs[0];
    m_Player->SetFPSCamera(camDesc);

    // init the renderer
    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->SetInstanced(setInstanced);

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
    // m_FlyCamera->OnEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) { return false; });
    dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) { return false; });
    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) { return false; });
}

void GameLayer::OnUpdate(gdp1::Timestep ts) {
    if (setRunAnimation != previousRunAnimationState) {
        if (setRunAnimation)
            zombie1->SetCurrentAnimation("zombie_running");
        else
            zombie1->SetCurrentAnimation("zombie_crawl");

        // Update the previous state to the current state
        previousRunAnimationState = setRunAnimation;
    }

    if (setRunAnimation1 != previousRunAnimationState1) {
        if (setRunAnimation1)
            zombie2->SetCurrentAnimation("zombie_running");
        else
            zombie2->SetCurrentAnimation("zombie_walking");

        // Update the previous state to the current state
        previousRunAnimationState1 = setRunAnimation1;
    }

    // m_FlyCamera->OnUpdate(ts);
    m_Player->Update(ts);

    m_Physics->FixedUpdate(ts);
    m_Scene->Update(ts);

    // m_ParticleSystem->Render();

    m_Renderer->Render(m_Scene, m_Player->fps_camera_ptr_.get()->GetCamera(), ts, enableSkyBox, setInstanced);

    // Render Debug Boxes
    if (drawDebug) m_Renderer->RenderDebug(m_Scene, m_Player->fps_camera_ptr_.get()->GetCamera(), ts, enableSkyBox);
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

    ImGui::Checkbox("Enable Skybox", &enableSkyBox);
    ImGui::Checkbox("Draw Debug", &drawDebug);
    ImGui::Checkbox("Set Instanced", &setInstanced);
    ImGui::Checkbox("Set Run Animation", &setRunAnimation);
    ImGui::Checkbox("Set Run Animation Zombie 1", &setRunAnimation1);
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

        /*PointLight pointLight;
        pointLight.color = randomColor;
        pointLight.name = desc.name;
        pointLight.position = {x, y, z};
        pointLight.intensity = 0.7f;
        pointLight.constant = 0.5f;
        pointLight.linear = 2.0f;
        pointLight.quadratic = 1.0f;

        scene->AddPointLight(pointLight);*/

        // m_Scene->GetLevelDesc().rigidbodyDescs.push_back(rigidbodyDesc);

        scene->AddGameObject(raindrop);  // Add the raindrop to the scene (assuming this function exists)
    }
}

void GameLayer::AddPlayer() {
    GameObjectDesc playerDesc;

    playerDesc.name = "Player";
    playerDesc.modelName = "PlayerHands";
    playerDesc.visible = true;
    playerDesc.transform.localPosition = glm::vec3(0.f, 0.3f, -13.f);
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

    // GameObjectDesc weaponDesc;

    // weaponDesc.name = "ArGun";
    // weaponDesc.modelName = "AR_Gun";
    // weaponDesc.visible = true;
    // weaponDesc.transform.localPosition = glm::vec3(0.f, 1.f, 0.f);
    // weaponDesc.transform.localScale = glm::vec3(0.05f);
    // weaponDesc.transform.localEulerAngles = glm::vec3(0.0f, 0.f, 0.f);
    // weaponDesc.setLit = true;
    // weaponDesc.parentName = "";

    m_Player = new Player(m_Scene.get(), playerDesc);
    m_Player->model = m_Scene->FindModelByName(playerDesc.modelName);
    m_Player->SetCurrentAnimation("Player_Idle");

    m_Scene->AddGameObject(m_Player);
    // m_Scene->AddGameObject(new Weapon(m_Scene.get(), weaponDesc, WeaponType::RIFLE));
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
