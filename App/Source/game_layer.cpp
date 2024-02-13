#include "game_layer.h"

GameLayer::GameLayer()
    : Layer("Game") {}

void GameLayer::OnAttach() {}

void GameLayer::OnDetach() {}

void GameLayer::OnUpdate(gdp1::Timestep ts) { LOG_INFO("GameLayer::OnUpdate"); }

void GameLayer::OnImGuiRender() {}

void GameLayer::OnEvent(gdp1::Event& event) { LOG_TRACE("{0}", event); }
