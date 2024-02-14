#include "player.h"
#include "Utils/GameConstants.h"

namespace Game {

Player::Player(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc)
    : gdp1::GameObject(scn, desc)
    , health(DEFAULT_PLAYER_HEALTH)
    , movement_speed(DEFAULT_PLAYER_MOVEMENT_SPEED)
    , current_selected_weapon(PRIMARY_WEAPON)
    , primary_weapon(nullptr)
    , secondary_weapon(nullptr)
    , melee_weapon(nullptr) {}

Player::Player(gdp1::Scene* scn, const std::string& name)
    : gdp1::GameObject(scn, name)
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

}  // namespace Game