#pragma once

#include <memory>

namespace gdp1 {

// Forward declarations
class Scene;
class Camera;

class Renderer {
public:
    Renderer() = default;
    void Render(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, bool renderSkybox = true);

private:
    void ResetFrameBuffers();
};

}  // namespace gdp1
