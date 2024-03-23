#pragma once

#include "common.h"
#include "Physics/softbody.h"
#include "Core/transform.h"
#include "Core/game_object.h"
#include "Render/model.h"
#include "Render/scene.h"
#include "Resource/level_object_description.h"

namespace gdp1 {

class SoftBodyUtils {
public:

    static SoftBody* CreateSoftBody(SoftbodyDesc& softbodyDesc, GameObject* go);

    static SoftBody* CreateChain(glm::vec3 startPos, unsigned int chainSize, float spacing);

    static void CreateBallSprings(SoftBody* body, float radius, int latitudeSegments, int longitudeSegments,
                                  float stiffness);

    static void AddChain(GameObject* go, std::vector<GameObject*> chainObjects, unsigned int chainSize,
                         float spacing, int attachVertexIndex);

    static SoftBody* CreatePlatform(Scene* scene, std::vector<GameObject*>& platformObjects, glm::vec3 startPos,
                                    unsigned int chainSize, float spacing);
};

}  // namespace gdp1