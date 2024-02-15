#include "Core/game_layer.h"

using namespace gdp1;

class Game : public Application {
public:
    Game() { PushLayer(new GameLayer()); }
};

int main(int argc, char* argv[]) {
    std::unique_ptr<Game> game = std::make_unique<Game>();
    game->Run();
    return 0;
}
