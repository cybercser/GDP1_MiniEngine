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

    // init physics engine
    m_Physics = std::make_unique<Physics>(m_Scene.get(), levelJson);

    animatedModel = m_Scene->FindModelByName("Fall_Flat");

    GameObject* cloth = m_Scene->FindObjectByName("Cloth");
    if (cloth && cloth->hasSoftBody) {
        //cloth->softBody->ApplyForce(glm::vec3(0.0f, 0.f, 0.5f));
        cloth->transform->localPosition = (glm::vec3(0.f, 20.f, 0.0f));

        cloth->softBody->CreateRandomSprings(100, 1.0f);
        cloth->softBody->particles[0]->isPinned = true;
        //cloth->softBody->particles[0]->position.z -= 2.0;

        cloth->softBody->particles[525]->isPinned = true;
        //cloth->softBody->particles[271]->position.z += 2.0;
    }

    GameObject* cube1 = m_Scene->FindObjectByName("Chomp");
    if (cube1 && cube1->hasSoftBody) {
        cube1->transform->SetPosition(glm::vec3(10.f, 10.f, 10.f));
        cube1->softBody->CreateRandomSprings(2000, 1.0f);
        AddChainToSoftBody(cube1, 5, 0.2f, 108);

        movableParticle = cube1->softBody->particles[cube1->softBody->particles.size() - 1];
    }

    GameObject* blaster = m_Scene->FindObjectByName("Blaster");
    if (blaster && blaster->hasSoftBody) {
        blaster->transform->SetPosition(glm::vec3(30.f, 10.f, 0.0f));
        blaster->softBody->CreateRandomSprings(50000, 0.5f);
    }

    GameObject* ball = m_Scene->FindObjectByName("Ball");
    if (ball && ball->hasSoftBody) {
        ball->softBody->CreateRandomSprings(10000, 0.1f);
    }

    GameObject* ball1 = m_Scene->FindObjectByName("Ball1");
    if (ball1 && ball1->hasSoftBody) {
        ball1->softBody->CreateRandomSprings(8000, 0.05f);
    }

    CreateSoftBodyBridge(20, glm::vec3(10.0f, 5.0f, 11.0f), 0.3f, "bridge_left");

    m_Physics->StartSoftBodyThreads();

    // configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

    if (animatedModel != nullptr) {
        if (Input::IsKeyPressed(HZ_KEY_1)) {
            animatedModel->SetCurrentAnimation("Fall_Flat");
        }
        if (Input::IsKeyPressed(HZ_KEY_2)) {
            animatedModel->SetCurrentAnimation("Chapa");
        }
        if (Input::IsKeyPressed(HZ_KEY_3)) {
            animatedModel->SetCurrentAnimation("Standing_Taunt");
        }
        if (Input::IsKeyPressed(HZ_KEY_4)) {
            animatedModel->SetCurrentAnimation("Jumping");
        }
        if (Input::IsKeyPressed(HZ_KEY_5)) {
            animatedModel->SetCurrentAnimation("Jumping_Down");
        }
    }

    if (movableObject != nullptr) {
        if (Input::IsKeyPressed(HZ_KEY_UP) && !Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.z += 0.1f;
            movableObject->transform->SetPosition(newPos);

            movableParticle->oldPosition = movableParticle->position;
            movableParticle->position = newPos;
        }
        if (Input::IsKeyPressed(HZ_KEY_LEFT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.x += 0.1f;
            movableObject->transform->SetPosition(newPos);
            movableParticle->oldPosition = movableParticle->position;
            movableParticle->position = newPos;
        }
        if (Input::IsKeyPressed(HZ_KEY_RIGHT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.x -= 0.1f;
            movableObject->transform->SetPosition(newPos);
            movableParticle->oldPosition = movableParticle->position;
            movableParticle->position = newPos;
        }
        if (Input::IsKeyPressed(HZ_KEY_DOWN) && !Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.z -= 0.1f;
            movableObject->transform->SetPosition(newPos);
            movableParticle->oldPosition = movableParticle->position;
            movableParticle->position = newPos;
        }
        if (Input::IsKeyPressed(HZ_KEY_UP) && Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.y += 0.1f;
            movableObject->transform->SetPosition(newPos);
            movableParticle->oldPosition = movableParticle->position;
            movableParticle->position = newPos;
        }
        if (Input::IsKeyPressed(HZ_KEY_DOWN) && Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.y -= 0.1f;
            movableObject->transform->SetPosition(newPos);
            movableParticle->oldPosition = movableParticle->position;
            movableParticle->position = newPos;
        }
        if (Input::IsKeyPressed(HZ_KEY_F)) {
            bridgeRopeLeft->springs[7]->isActive = false;
            bridgeRopeRight->springs[7]->isActive = false;

            bridgeRopeLeft->springs[3]->isActive = false;
            bridgeRopeRight->springs[3]->isActive = false;
        }
        if (Input::IsKeyPressed(HZ_KEY_G)) {
            bridgeRopeLeft->springs[7]->isActive = true;
            bridgeRopeRight->springs[7]->isActive = true;
            bridgeRopeLeft->springs[3]->isActive = true;
            bridgeRopeRight->springs[3]->isActive = true;
        }
    }

    bridgeRopeLeft->Update(ts);
    bridgeRopeRight->Update(ts);
    // platform->Update(ts);

    for (unsigned int i = 0; i < bridgePlatforms.size(); i++) {
        GameObject* go = bridgePlatforms[i];
        
        go->softBody->particles[2]->position = bridgeRopeLeft->particles[i * 2]->position;
        go->softBody->particles[4]->position = bridgeRopeLeft->particles[(i * 2) + 1]->position;

        go->softBody->particles[1]->position = bridgeRopeRight->particles[i * 2]->position;
        go->softBody->particles[6]->position = bridgeRopeRight->particles[(i * 2) + 1]->position;

        go->softBody->Update(ts);
    }

    m_Physics->FixedUpdate(ts);
    m_Scene->Update(ts);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_Renderer->Render(m_Scene, m_FlyCamera->GetCamera());
}

void GameLayer::OnImGuiRender() {
    ImGui::Begin("Controls");
    ImGui::Text("WASD to move, mouse to look around");
    ImGui::Text("F - Destroy Bridge, G - Rebuild Bridge");
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

gdp1::SoftBody* GameLayer::CreateChain(glm::vec3 startPos, int chainSize, float chainSpacing, std::string name) {
    gdp1::SoftBody* chain = SoftBodyUtils::CreateChain(startPos, chainSize, chainSpacing);
    Model* sphere_model = m_Scene->FindModelByName("ChainSphere");

    for (int i = 0; i <= chainSize; i++) {
        TransformDesc transformDesc;
        transformDesc.localPosition = startPos;
        transformDesc.localEulerAngles = glm::vec3(0.0f);
        transformDesc.localScale = glm::vec3(0.5f);

        GameObjectDesc newObjectDesc;
        newObjectDesc.name = name + "_chain_" + std::to_string(i);
        newObjectDesc.modelName = "ChainSphere";
        newObjectDesc.transform = transformDesc;

        GameObject* go = new GameObject(m_Scene.get(), newObjectDesc);
        go->model = sphere_model;
        go->visible = true;

        chain->particles[i]->go = go;

        m_Scene->AddGameObject(go);
    }

    return chain;
}

void GameLayer::AddChainToSoftBody(gdp1::GameObject* gameObject, int chainSize, float chainSpacing,
                                   int attachVertexIndex) {
    std::vector<GameObject*> chainObjects;
    Model* sphere_model = m_Scene->FindModelByName("ChainSphere");
    for (int i = 0; i <= chainSize; i++) {
        TransformDesc transformDesc;
        transformDesc.localPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        transformDesc.localEulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
        transformDesc.localScale = glm::vec3(1.0f, 1.0f, 1.0f);

        GameObjectDesc newObjectDesc;
        newObjectDesc.name = gameObject->name + "_chain_" + std::to_string(i);
        newObjectDesc.modelName = "ChainSphere";
        newObjectDesc.transform = transformDesc;

        GameObject* go = new GameObject(m_Scene.get(), newObjectDesc);
        go->model = sphere_model;
        go->visible = true;

        m_Scene->AddGameObject(go);
        chainObjects.push_back(go);

        if (i == chainSize) {
            movableObject = go;
        }
    }

    SoftBodyUtils::AddChain(gameObject, chainObjects, chainSize, chainSpacing, attachVertexIndex);
}

void GameLayer::CreateSoftBodyBridge(int chainSize, glm::vec3 startPos, float chainSpacing, std::string name) {
    Model* woodModel = m_Scene->FindModelByName("Wood");

    glm::vec3 startLeftPos = startPos;
    startLeftPos.z += 2.0f;

    bridgeRopeLeft = CreateChain(startLeftPos, chainSize, chainSpacing, "bridge_left");
    bridgeRopeRight = CreateChain(startPos, chainSize, chainSpacing, "bridge_right");

    for (unsigned int i = 0; i < chainSize; i += 2) {
        TransformDesc transformDesc;
        transformDesc.localPosition = glm::vec3(startPos.x - (i * chainSpacing), startPos.y, startPos.z);
        transformDesc.localEulerAngles = glm::vec3(90.0f, 0.0f, 0.0f);
        transformDesc.localScale = glm::vec3(1.0f, 1.0f, 1.0f);

        GameObjectDesc bodyDesc;
        bodyDesc.name = "platform_" + std::to_string(i);
        bodyDesc.modelName = "Wood";
        bodyDesc.transform = transformDesc;

        GameObject* go = new GameObject(m_Scene.get(), bodyDesc);
        go->model = woodModel;
        go->visible = true;

        SoftbodyDesc softBodyDesc;
        softBodyDesc.iterations = 5;
        softBodyDesc.mass = 1.0f;
        softBodyDesc.objectName = bodyDesc.name;
        softBodyDesc.springStrength = 1.0f;

        go->softBody = SoftBodyUtils::CreateSoftBody(softBodyDesc, go);
        go->hasSoftBody = true;

        m_Scene->AddGameObject(go);
        bridgePlatforms.push_back(go);
    }
}
