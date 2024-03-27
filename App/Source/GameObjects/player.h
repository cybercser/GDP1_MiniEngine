#pragma once

#include <Core/game_object.h>

#include "weapon.h"
#include <Resource/level_object_description.h>
#include <Utils/fps_camera_controller.h>
#include <Utils/camera.h>

enum WeaponSelection { PRIMARY_WEAPON, SECONDARY_WEAPON, MELEE_WEAPON };

class Player : public gdp1::GameObject {

class Camera;

public:

    Player(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc);

    virtual void Update(float dt) override;
    virtual void OnCollision(gdp1::Contact* contact) override;
    virtual void OnEvent(gdp1::Event& event) override;

    // Set 
    void SetHealth(int health);
    void TakeDamage(int damage);

    // Set primary, secondary, and melee weapons
    void SetPrimaryWeapon(Weapon* primary_weapon);
    void SetSecondaryWeapon(Weapon* secondary_weapon);
    void SetMeleeWeapon(Weapon * melee_weapon);

    // Change Weapon Selection
    void ChangeWeapon(WeaponSelection selection);

    // Use actions
    void Shoot();
    void Collect();
    void Sprint();
    void Reload();

    void SetFPSCamera(const gdp1::CameraDesc& camDesc);
    bool OnMouseMoved(gdp1::MouseMovedEvent& e);

    std::shared_ptr<gdp1::FPSCameraController> fps_camera_ptr_;

private:
    // Additional player-specific attributes and methods can be added here

    int health;
    float movement_speed;

    WeaponSelection current_selected_weapon;

    Weapon* primary_weapon;
    Weapon* secondary_weapon;
    Weapon* melee_weapon;

    glm::vec3 m_Forward = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_Right = glm::vec3(-1.0f, 0.0f, 0.0f);

    float m_LastX;
    float m_LastY;

    bool firstMouse = true;

 private:
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);
    void ProcessKeyboardInput(gdp1::CameraMovement movementDirection, float dt);
};
