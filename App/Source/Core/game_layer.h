#pragma once

#include <engine.h>
#include <engine_utils.h>
#include <Render/model.h>
#include <Physics/physics.h>

class GameLayer : public gdp1::Layer {

class SoftBody;

public:
    GameLayer();
    virtual ~GameLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnEvent(gdp1::Event& event) override;
    virtual void OnUpdate(gdp1::Timestep ts) override;
    virtual void OnImGuiRender() override;

private:
    std::shared_ptr<gdp1::Scene> m_Scene;
    std::shared_ptr<gdp1::FlyCameraController> m_FlyCamera;
    std::unique_ptr<gdp1::Renderer> m_Renderer;
    std::unique_ptr<gdp1::Physics> m_Physics;

    gdp1::Model* animatedModel = nullptr;

    gdp1::GameObject* movableObject;

    // GameUtils

    void AddChainToSoftBody(gdp1::GameObject* gameObject, int chainSize, float chainSpacing);
};
