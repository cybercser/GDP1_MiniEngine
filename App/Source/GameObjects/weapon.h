#pragma once

#include <Core/game_object.h>

enum WeaponType { PISTOL, RIFLE, MISSILE_LAUNCHER };

class Weapon : public gdp1::GameObject {

public:
    Weapon(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc, WeaponType weapon_type);
    Weapon(gdp1::Scene* scn, const std::string& name, WeaponType weapon_type);

    virtual void Update(float dt) override;
    virtual void OnCollision(gdp1::Contact* contact) override;
    //virtual void OnEvent(gdp1::Event& event) override;

    // Weapon Controls
    void Shoot();

    void Reload();

    void Upgrade(); // Weapon can be upgraded by player using collectibles from the inventory

    void Attack(); // Can be used only for melee weapon

    void SetWeaponType(WeaponType weapon_type); // Change weapon type

private:
    // Additional weapon-specific attributes and methods can be added here
    WeaponType weapon_type;
    
    int ammo_capacity;
    int reload_speed;
    int damage;
    int ammunition;
    int firing_delay;
    int reload_capacity;

};
