#pragma once

#include <engine.h>
#include <engine_utils.h>
#include <Render/model.h>
#include <Physics/physics.h>
#include <Physics/softbody.h>
#include <Audio/audio_manager.h>
#include <Core/game_object.h>

#include "IO/sqlite_database.h"

#include "GameplayManager/GameplayManager.h"
#include "GameObjects/player.h"
#include "GameObjects/collectible.h"

class GameLayer : public gdp1::Layer {

    class SoftBody;
    class SoftBodyParticle;

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
    std::unique_ptr<gdp1::AudioManager> m_audioManager;

    Player* m_Player;
    gdp1::GameObject* zombie1;
    gdp1::GameObject* zombie2;
    GameplayManager* gameplayManager;

    std::shared_ptr<gdp1::SQLiteDatabase> db;

    bool enableSkyBox = false;
    bool setRunAnimation = false;
    bool setRunAnimation1 = false;
    bool previousRunAnimationState = false;
    bool previousRunAnimationState1 = false;
    bool drawDebug = false;

    void CreateRaindropObjects(gdp1::Scene* scene, int numRaindrops);

    void AddPlayer();
    void AddCoins();

};
