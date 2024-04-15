#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace gdp1 {

class GameObject;
class Camera;

class LODSystem {
public:
    LODSystem() = default;

    void Update(std::shared_ptr<Camera> camera, std::unordered_map<std::string, GameObject*>& gameObjects);
};

}  // namespace gdp1