#pragma once

#include <Core/game_object.h>

#include "weapon.h"

enum WeaponSelection { PRIMARY_WEAPON, SECONDARY_WEAPON, MELEE_WEAPON };

class Player : public gdp1::GameObject {

public:

    Player(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc);

    virtual void Update(float dt) override;
    virtual void OnCollision(gdp1::Contact* contact) override;

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

private:
    // Additional player-specific attributes and methods can be added here

    int health;
    float movement_speed;

    WeaponSelection current_selected_weapon;

    Weapon* primary_weapon;
    Weapon* secondary_weapon;
    Weapon* melee_weapon;

};
