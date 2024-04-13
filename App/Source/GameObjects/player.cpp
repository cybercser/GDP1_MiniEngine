#include "player.h"

#include <glm/gtx/vector_angle.hpp>

#include "Input/input.h"
#include "Input/key_codes.h"
#include "Input/mouse_button_codes.h"
#include "Utils/GameConstants.h"

Player::Player(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc)
    : gdp1::GameObject(scn, desc)
    , health(DEFAULT_PLAYER_HEALTH)
    , movement_speed(DEFAULT_PLAYER_MOVEMENT_SPEED)
    , current_selected_weapon(PRIMARY_WEAPON)
    , primary_weapon(nullptr)
    , secondary_weapon(nullptr)
    , melee_weapon(nullptr) {}

void Player::SetHealth(int health) { this->health = health; }

void Player::TakeDamage(int damage) { this->health = std::max(0, this->health - damage); }

void Player::Shoot() {}

void Player::SetPrimaryWeapon(Weapon* primary_weapon) { this->primary_weapon = primary_weapon; }

void Player::SetSecondaryWeapon(Weapon* secondary_weapon) { this->secondary_weapon = secondary_weapon; }

void Player::SetMeleeWeapon(Weapon* melee_weapon) { this->melee_weapon = melee_weapon; }

void Player::ChangeWeapon(WeaponSelection selection) { this->current_selected_weapon = selection; }

void Player::Collect() {}

void Player::Sprint() {}

void Player::Reload() {}

void Player::SetFPSCamera(const gdp1::CameraDesc& camDesc) {
    fps_camera_ptr_ = std::make_shared<gdp1::FPSCameraController>(camDesc, 16.0f / 9.0f, 100.0f, 100.0f);
}

void Player::Update(float ts) {
    if (gdp1::Input::IsKeyPressed(HZ_KEY_W)) {
        ProcessKeyboardInput(gdp1::CameraMovement::kFORWARD, ts);
    }
    if (gdp1::Input::IsKeyPressed(HZ_KEY_S)) {
        ProcessKeyboardInput(gdp1::CameraMovement::kBACKWARD, ts);
    }
    if (gdp1::Input::IsKeyPressed(HZ_KEY_A)) {
        ProcessKeyboardInput(gdp1::CameraMovement::kLEFT, ts);
    }
    if (gdp1::Input::IsKeyPressed(HZ_KEY_D)) {
        ProcessKeyboardInput(gdp1::CameraMovement::kRIGHT, ts);
    }

    glm::vec3 newPos = transform->localPosition;

    newPos.y += 0.7f;
    // newPos.z -= 0.5f;

    glm::vec3 center = newPos + m_Forward;
    fps_camera_ptr_.get()->GetCamera().get()->SetEyeCenter(newPos, center);
    fps_camera_ptr_.get()->SetPosition(newPos);
}

void Player::OnCollision(gdp1::Contact* contact) {}

void Player::OnEvent(gdp1::Event& event) {
    gdp1::EventDispatcher dispatcher(event);
    dispatcher.Dispatch<gdp1::MouseMovedEvent>(GLCORE_BIND_EVENT_FN(Player::OnMouseMoved));
}

void Player::ProcessKeyboardInput(gdp1::CameraMovement movementDirection, float dt) {
    float speed = DEFAULT_PLAYER_MOVEMENT_SPEED * dt;
    glm::vec3 translation_{0.0f, 0.0f, 0.0f};
    switch (movementDirection) {
        case gdp1::CameraMovement::kFORWARD:
            translation_ = m_Forward * speed;
            break;
        case gdp1::CameraMovement::kBACKWARD:
            translation_ = -m_Forward * speed;
            break;
        case gdp1::CameraMovement::kLEFT:
            translation_ = -m_Right * speed;
            break;
        case gdp1::CameraMovement::kRIGHT:
            translation_ = m_Right * speed;
            break;
    }

    glm::vec3 newPos = transform->localPosition += translation_;

    transform->SetPosition(newPos);
}

bool Player::OnMouseMoved(gdp1::MouseMovedEvent& e) {
    if (firstMouse) {
        m_LastX = e.GetX();
        m_LastY = e.GetY();
        firstMouse = false;
    }

    float xoffset = e.GetX() - m_LastX;
    float yoffset = m_LastY - e.GetY();

    m_LastX = e.GetX();
    m_LastY = e.GetY();

    ProcessMouseMovement(xoffset, yoffset, true);
    return false;  // event is not handled
}

void Player::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    //xOffset *= sensitivity;
    //yOffset *= sensitivity;

    //player->transform->rotation.y += xOffset;
    //player->transform->rotation.x -= yOffset;

    //if (constrainPitch) {
    //    if (player->transform->rotation.x > 89.0f) player->transform->rotation.x = 89.0f;
    //    if (player->transform->rotation.x < -89.0f) player->transform->rotation.x = -89.0f;
    //}

    //// Update player's forward and right vectors based on rotation
    //player->m_Forward = glm::normalize(glm::vec3(cos(glm::radians(player->transform->rotation.y)), 0.0f,
    //                                             sin(glm::radians(player->transform->rotation.y))));

    //player->m_Right = glm::normalize(glm::cross(player->m_Forward, glm::vec3(0.0f, 1.0f, 0.0f)));
}