#include "lod_system.h"

#include <Core/game_object.h>
#include <Render/model.h>
#include <Utils/camera.h>

using namespace std;

namespace gdp1 {

void LODSystem::Update(shared_ptr<Camera> camera, unordered_map<string, GameObject*>& gameObjects) {
#pragma omp parallel for
    for (unordered_map<string, GameObject*>::iterator it = gameObjects.begin(); it != gameObjects.end(); it++) {
        Model* model = it->second->model;
        if (model->lodLevels.size() <= 1) {
            model->currentLODLevel = 0;
            continue;
        }

        float distance = glm::length(camera->GetEye() - it->second->transform->localPosition);
        if (distance >= 0.f && distance <= 5.0f) {
            model->currentLODLevel = 0;
        } else {
            model->currentLODLevel = 1;
        }
    }
};

}  // namespace gdp1