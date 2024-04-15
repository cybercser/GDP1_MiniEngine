#include "Core/game_layer.h"

using namespace gdp1;

class Game : public Application {
public:
    Game() { PushLayer(new GameLayer()); }
};

int main(int argc, char* argv[]) {
    float lastTime = GetTickCount();
    std::unique_ptr<Game> game = std::make_unique<Game>();
    float currentTime = GetTickCount();
    LOG_ERROR("Time: {0}", currentTime - lastTime);
    game->Run();
    return 0;
}
