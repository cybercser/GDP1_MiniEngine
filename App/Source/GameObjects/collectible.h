#pragma once

#include "Core\game_object.h"

enum CollectibleType { COIN, HEALTH_KIT, AMMO, WEAPON_PART };

class Collectible : public gdp1::GameObject {
public:
    Collectible(gdp1::Scene* scn, const gdp1::GameObjectDesc& desc, CollectibleType collectible_type);
    Collectible(gdp1::Scene* scn, const std::string& name, CollectibleType collectible_type);

    virtual void Update(float dt) override;
    virtual void OnCollision(gdp1::Contact* contact) override;

private:
    CollectibleType collectible_type;
};
