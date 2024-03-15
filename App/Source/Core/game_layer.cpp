#include "game_layer.h"

using namespace gdp1;
using namespace gdp1::utils;

#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"

#include "Physics/softbody.h"
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
    if (cloth) {
        cloth->transform->localPosition = (glm::vec3(0.f, 50.f, 0.0f));
        // cloth->softBody->CreateRandomSprings(0);
        cloth->softBody->particles[0]->isPinned = true;
        cloth->softBody->particles[0]->position.z -= 1.0;
        cloth->softBody->particles[271]->isPinned = true;
        cloth->softBody->particles[271]->position.z += 1.0;
    }

    GameObject* cube = m_Scene->FindObjectByName("Cube");
    if (cube && cube->hasSoftBody) {
        cube->softBody->CreateRandomSprings(10);
    }

    GameObject* cube1 = m_Scene->FindObjectByName("Cube1");
    if (cube1 && cube1->hasSoftBody) {
        cube1->softBody->CreateRandomSprings(10);
        AddChainToSoftBody(cube1, 5, 1.5f);
    }

    GameObject* blaster = m_Scene->FindObjectByName("Blaster");
    if (blaster && blaster->hasSoftBody) {
        blaster->transform->SetPosition(glm::vec3(30.f, 10.f, 0.0f));
        //blaster->softBody = SoftBodyUtils::CreateChain(blaster->transform, 4, 1.0f);
        blaster->softBody->CreateRandomSprings(30000);
    }

    movableObject = m_Scene->FindObjectByName("Cube1_chain_4");
    

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
        if (Input::IsKeyPressed(HZ_KEY_UP)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.z += 2.0f;
            movableObject->transform->SetPosition(newPos);
        }
        if (Input::IsKeyPressed(HZ_KEY_LEFT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.x += 0.1f;
            movableObject->transform->SetPosition(newPos);
        }
        if (Input::IsKeyPressed(HZ_KEY_RIGHT)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.x -= 0.1f;
            movableObject->transform->SetPosition(newPos);
        }
        if (Input::IsKeyPressed(HZ_KEY_DOWN)) {
            glm::vec3 newPos(movableObject->transform->localPosition);
            newPos.z -= 0.1f;
            movableObject->transform->SetPosition(newPos);
        }
    }

    m_Physics->FixedUpdate(ts);
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

void GameLayer::AddChainToSoftBody(gdp1::GameObject* gameObject, int chainSize, float chainSpacing) {
    std::vector<GameObject*> chainObjects;
    Model* sphere_model = m_Scene->FindModelByName("sphere");
    for (int i = 0; i <= chainSize; i++) {
        TransformDesc transformDesc;
        transformDesc.localPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        transformDesc.localEulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
        transformDesc.localScale = glm::vec3(1.0f, 1.0f, 1.0f);

        GameObjectDesc newObjectDesc;
        newObjectDesc.name = gameObject->name + "_chain_" + std::to_string(i);
        newObjectDesc.modelName = "sphere"; 
        newObjectDesc.transform = transformDesc;

        GameObject* go = new GameObject(m_Scene.get(), newObjectDesc);
        go->model = sphere_model;
        go->visible = true;

        m_Scene->AddGameObject(go);
        chainObjects.push_back(go);
    }

    SoftBodyUtils::AddChain(gameObject, chainObjects, chainSize, chainSpacing);
}
