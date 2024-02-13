#pragma once

#include <common.h>
#include <Core/layer.h>
#include <Core/application.h>

class GameLayer : public gdp1::Layer {
public:
    GameLayer();
    virtual ~GameLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(gdp1::Timestep ts) override;
    virtual void OnImGuiRender() override;
    virtual void OnEvent(gdp1::Event& event) override;
};
