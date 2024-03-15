#pragma once

#include "common.h"
#include "Physics/softbody.h"
#include "Core/transform.h"
#include "Core/game_object.h"
#include "Render/model.h"

namespace gdp1 {

class SoftBodyUtils {
public:
    static SoftBody* CreateChain(Transform* transform, unsigned int chainSize, float spacing);

    static void AddChain(GameObject* go, std::vector<GameObject*> chainObjects, unsigned int chainSize, float spacing);
};

}  // namespace gdp1