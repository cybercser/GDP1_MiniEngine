#include "weapon.h"
#include "Utils/GameConstants.h"

namespace Game {

int ammo_capacity;
int reload_speed;
int damage;
int ammunition;
int firing_delay;

Weapon::Weapon(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc, WeaponType weapon_type)
    : gdp1::GameObject(scn, desc)
    , weapon_type(weapon_type)
    , ammo_capacity(DEFAULT_WEAPON_AMMO_CAPACITY)
    , reload_speed(DEFAULT_WEAPON_RELOAD_SPEED)
    , damage(DEFAULT_WEAPON_DAMAGE)
    , ammunition(DEFAULT_WEAPON_AMMUNITION)
    , firing_delay(DEFAULT_WEAPON_FIRING_DELAY)
    , reload_capacity(DEFAULT_WEAPON_RELOAD_CAPACITY) {}

Weapon::Weapon(gdp1::Scene* scn, const std::string& name, WeaponType weapon_type)
    : gdp1::GameObject(scn, name)
    , weapon_type(weapon_type)
    , ammo_capacity(DEFAULT_WEAPON_AMMO_CAPACITY)
    , reload_speed(DEFAULT_WEAPON_RELOAD_SPEED)
    , damage(DEFAULT_WEAPON_DAMAGE)
    , ammunition(DEFAULT_WEAPON_AMMUNITION)
    , firing_delay(DEFAULT_WEAPON_FIRING_DELAY)
    , reload_capacity(DEFAULT_WEAPON_RELOAD_CAPACITY) {}


void Weapon::Shoot() {}

void Weapon::Reload() {}

void Weapon::Upgrade() {}

void Weapon::Attack() {}

void Weapon::SetWeaponType(WeaponType weapon_type) {}

}